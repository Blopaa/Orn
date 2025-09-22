/**
 * @file codeGeneration.c
 * @brief Main code generation implementation for the C compiler.
 *
 * Implements the complete code generation phase that transforms Abstract
 * Syntax Trees into x86-64 assembly code. Handles all language constructs
 * including variable declarations, expressions, control flow, and proper
 * register allocation and stack management.
 *
 * Key features:
 * - Complete AST traversal and code emission
 * - Register allocation and management
 * - Stack-based variable storage
 * - Expression evaluation with proper operator precedence
 * - Control flow generation (conditionals, loops)
 * - Type-aware code generation for int, float, bool, and string types
 * - Memory management and context cleanup
 */

#include "codeGeneration.h"

#include <stdlib.h>
#include <string.h>

#include "asmTemplate.h"
#include "builtIns.h"
#include "errorHandling.h"

/**
 * @brief Generates assembly code for a single AST node and its subtree.
 *
 * Main dispatch function for code generation that handles all AST node types.
 * Recursively processes the AST structure and emits appropriate assembly code
 * for each language construct. Manages context state and error propagation
 * throughout the generation process.
 *
 * Supported node types:
 * - PROGRAM: Root program structure
 * - Variable declarations: INT/FLOAT/STRING/BOOL_VARIABLE_DEFINITION
 * - Assignments: ASSIGNMENT and compound assignments (+=, -=, etc.)
 * - Control flow: IF_CONDITIONAL, LOOP_STATEMENT
 * - Block statements: BLOCK_STATEMENT, BLOCK_EXPRESSION
 * - Expression statements: Arithmetic, logical, comparison operations
 *
 * @param node AST node to generate code for
 * @param context Code generation context with output file and symbol info
 * @return 1 on success, 0 on failure
 *
 * @note This function maintains the call stack structure and ensures
 *       proper code generation order. Error conditions are propagated
 *       upward to allow for graceful failure handling.
 */
int generateNodeCode(ASTNode node, StackContext context) {
  if (node == NULL || context == NULL)
    return 0;

  switch (node->nodeType) {
  case PROGRAM: {
    ASTNode child = node->children;
    while (child != NULL) {
      if (child->nodeType == FUNCTION_DEFINITION) {
        generateNodeCode(child, context);
        context->currentOffset = 0; // Reset for next function
      }
      child = child->brothers;
    }

    // generate main
    fprintf(context->file, "\n.globl main\n");
    fprintf(context->file, "main:\n");
    fprintf(context->file, "%s\n", ASM_FUNCTION_PROLOGUE);

    // Generate main code (everything except functions)
    child = node->children;
    while (child != NULL) {
      if (child->nodeType != FUNCTION_DEFINITION) {
        if (!generateNodeCode(child, context))
          return 0;
      }
      child = child->brothers;
    }

    // Main epilogue
    fprintf(context->file, "\n");
    ASM_EMIT_COMMENT(context->file, "End of main function");
    fprintf(context->file, "%s\n", ASM_FUNCTION_EPILOGUE);
    fprintf(context->file, "    ret                  # Return to runtime\n");

    return 1;
  }

  case INT_VARIABLE_DEFINITION:
  case FLOAT_VARIABLE_DEFINITION:
  case STRING_VARIABLE_DEFINITION:
  case BOOL_VARIABLE_DEFINITION: {
    DataType varType = getDataTypeFromNode(node->nodeType);
    int offset = allocateVariable(context, node->start, node->length, varType);

    if (offset == 0)
      return 0;

    // Initialize with value if provided
    if (node->children != NULL) {
      emitComment(context, "Initialize variable");
      RegisterId initReg;
      if (varType == TYPE_FLOAT) {
        initReg =
            generateExpressionToRegister(node->children, context, REG_XMM0);
      } else {
        initReg =
            generateExpressionToRegister(node->children, context, REG_RAX);
      }
      generateStoreVariable(context, node->start, node->length, initReg, node);
    }
    return 1;
  }

  case ASSIGNMENT:
  case COMPOUND_ADD_ASSIGN:
  case COMPOUND_SUB_ASSIGN:
  case COMPOUND_MUL_ASSIGN:
  case COMPOUND_DIV_ASSIGN: {
    if (node->children == NULL || node->children->brothers == NULL)
      return 0;
    ASTNode leftNode = node->children;
    ASTNode rightNode = node->children->brothers;

    if (leftNode->nodeType != VARIABLE && leftNode->nodeType != MEMBER_ACCESS) {
      reportError(ERROR_INVALID_ASSIGNMENT_TARGET,
                  createErrorContextFromCodeGen(node, context),
                  "Left side must be variable or a member access");
      return 0;
    }
    if (leftNode->nodeType == MEMBER_ACCESS) {
      ASTNode objectNode = leftNode->children;
      ASTNode fieldNode = objectNode->brothers;

      StackVariable structVar =
          findStackVariable(context, objectNode->start, objectNode->length);
      if (!structVar || structVar->dataType != TYPE_STRUCT) {
        reportError(ERROR_INVALID_OPERATION_FOR_TYPE,
                    createErrorContextFromCodeGen(node, context),
                    "Member access on non-struct");
        return 0;
      }

      StructField field = findStructField(structVar->structType,
                                          fieldNode->start, fieldNode->length);
      if (!field) {
        reportError(ERROR_UNDEFINED_VARIABLE,
                    createErrorContextFromCodeGen(node, context),
                    "Struct field not found");
        return 0;
      }
      int memberOffset = structVar->stackOffset + field->offset;
      if (node->nodeType == ASSIGNMENT) {
        RegisterId rightReg;
        if (field->type == TYPE_FLOAT) {
          rightReg = generateExpressionToRegister(rightNode, context, REG_XMM0);
          fprintf(context->file,
                  "    movsd %s, %d(%%rbp)    # Store to struct member\n",
                  getFloatRegisterName(rightReg), memberOffset);
        } else {
          rightReg = generateExpressionToRegister(rightNode, context, REG_RAX);
          fprintf(context->file,
                  "    movq %s, %d(%%rbp)     # Store to struct member\n",
                  getRegisterName(rightReg, field->type), memberOffset);
        }
      }else {
        RegisterId leftReg, rightReg;
        if (field->type == TYPE_FLOAT) {
          leftReg = REG_XMM0;
          fprintf(context->file,
                  "    movsd %d(%%rbp), %s    # Load struct member\n",
                  memberOffset, getFloatRegisterName(leftReg));
          rightReg = REG_XMM1;
        } else {
          leftReg = REG_RAX;
          fprintf(context->file,
                  "    movq %d(%%rbp), %s     # Load struct member\n",
                  memberOffset, getRegisterName(leftReg, field->type));
          rightReg = REG_RBX;
        }
        rightReg = generateExpressionToRegister(rightNode, context, rightReg);

        NodeTypes opType;
        switch (node->nodeType) {
        case COMPOUND_ADD_ASSIGN:
          opType = ADD_OP;
          break;
        case COMPOUND_SUB_ASSIGN:
          opType = SUB_OP;
          break;
        case COMPOUND_MUL_ASSIGN:
          opType = MUL_OP;
          break;
        case COMPOUND_DIV_ASSIGN:
          opType = DIV_OP;
          break;
        default:
          opType = ADD_OP;
          break;
        }

        generateBinaryOp(context, opType, leftReg, rightReg, leftReg,
                         field->type, 0);

        if (field->type == TYPE_FLOAT) {
          fprintf(context->file,
                  "    movsd %s, %d(%%rbp)    # Store back to struct member\n",
                  getFloatRegisterName(leftReg), memberOffset);
        } else {
          fprintf(context->file,
                  "    movq %s, %d(%%rbp)     # Store back to struct member\n",
                  getRegisterName(leftReg, field->type), memberOffset);
        }
      }
    } else {
      StackVariable var =
          findStackVariable(context, leftNode->start, leftNode->length);
      if (var == NULL) {
        reportError(ERROR_UNDEFINED_VARIABLE,
                    createErrorContextFromCodeGen(node, context),
                    extractText(leftNode->start, leftNode->length));
        return 0;
      }
      RegisterId rightReg;
      if (var->dataType == TYPE_FLOAT) {
        rightReg = generateExpressionToRegister(rightNode, context, REG_XMM0);
      } else {
        rightReg = generateExpressionToRegister(rightNode, context, REG_RAX);
      }

      if (node->nodeType == ASSIGNMENT) {
        generateStoreVariable(context, leftNode->start, leftNode->length,
                              rightReg, node);
      } else {
        RegisterId leftReg;
        if (var->dataType == TYPE_FLOAT) {
          leftReg = REG_XMM1;
        } else {
          leftReg = REG_RBX;
        }

        generateLoadVariable(context, leftNode->start, leftNode->length,
                             leftReg, node);

        NodeTypes opType;
        switch (node->nodeType) {
        case COMPOUND_ADD_ASSIGN:
          opType = ADD_OP;
          break;
        case COMPOUND_SUB_ASSIGN:
          opType = SUB_OP;
          break;
        case COMPOUND_MUL_ASSIGN:
          opType = MUL_OP;
          break;
        case COMPOUND_DIV_ASSIGN:
          opType = DIV_OP;
          break;
        default:
          opType = ADD_OP;
          break;
        }

        generateBinaryOp(context, opType, leftReg, rightReg, leftReg,
                         var->dataType, 0);
        generateStoreVariable(context, leftNode->start, leftNode->length,
                              leftReg, node);
      }
    }

    return 1;
  }

  case BLOCK_STATEMENT:
  case BLOCK_EXPRESSION: {
    emitComment(context, "Block start");

    // Generate code for all statements in the block
    ASTNode child = node->children;
    while (child != NULL) {
      if (!generateNodeCode(child, context))
        return 0;
      child = child->brothers;
    }

    emitComment(context, "Block end");
    return 1;
  }

  case IF_CONDITIONAL: {
    return generateConditional(node, context);
  }

  case LOOP_STATEMENT: {
    return generateLoop(node, context);
  }
  case FUNCTION_DEFINITION: {
    // Generate function label
    char *tempVal = extractText(node->start, node->length);
    fprintf(context->file, "\n%s:\n", tempVal);
    if (tempVal)
      free(tempVal);
    fprintf(context->file, "    pushq %%rbp\n");
    fprintf(context->file, "    movq %%rsp, %%rbp\n");

    // Get nodes
    ASTNode paramList = node->children;
    ASTNode returnType = paramList ? paramList->brothers : NULL;
    ASTNode body = returnType ? returnType->brothers : NULL;

    // Save parameters to local variables
    if (paramList && paramList->nodeType == PARAMETER_LIST) {
      ASTNode param = paramList->children;
      RegisterId paramRegs[] = {REG_RDI, REG_RSI, REG_RDX,
                                REG_RCX, REG_R8,  REG_R9};
      int paramIndex = 0;

      while (param != NULL && paramIndex < 6) {
        DataType paramType = TYPE_INT; // Simplified
        int offset =
            allocateVariable(context, param->start, param->length, paramType);

        char *paramName = extractText(param->start, param->length);
        fprintf(context->file, "    movq %s, %d(%%rbp)    # Store param %s\n",
                getRegisterName(paramRegs[paramIndex], TYPE_INT), offset,
                paramName ? paramName : "unknown");
        if (paramName)
          free(paramName);

        param = param->brothers;
        paramIndex++;
      }
    }

    // Generate function body
    if (body != NULL) {
      generateNodeCode(body, context);
    }

    // Default epilogue (in case no return)
    fprintf(context->file, "    movq %%rbp, %%rsp\n");
    fprintf(context->file, "    popq %%rbp\n");
    fprintf(context->file, "    ret\n");

    return 1;
  }

  case RETURN_STATEMENT: {
    if (node->children != NULL) {
      // Force result into RAX for return
      generateExpressionToRegister(node->children, context, REG_RAX);
    }

    fprintf(context->file, "%s\n", ASM_FUNCTION_EPILOGUE);
    fprintf(context->file, "    ret\n");

    return 1;
  }
  case STRUCT_VARIABLE_DEFINITION: {
    ASTNode typeRef = node->children;
    StructType structType =
        findGlobalStructType(context, typeRef->start, typeRef->length);
    if (!structType) {
      reportError(ERROR_UNDEFINED_VARIABLE,
                  createErrorContextFromCodeGen(typeRef, context),
                  "Unknown struct type");
      return 0;
    }
    emitComment(context, "Struct var declaration");
    allocateStructVariable(context, node->start, node->length, structType);
    if (typeRef->brothers) {
      emitComment(context, "Struct var declaration");
      generateExpressionToRegister(typeRef->brothers, context, REG_RAX);
    }
    return 1;
  }
  case STRUCT_DEFINITION:
    return 1;
    break;
  // Expression statements (standalone expressions)
  default: {
    // For any other node type, try to generate it as an expression
    generateExpressionToRegister(node, context, REG_RAX);
    return 1;
  }
  }
}

/**
 * @brief Creates and initializes a code generation context.
 *
 * Allocates memory for the code generation context and opens the output
 * file for assembly code emission. Initializes all context fields including
 * variable tracking, string table, and generation counters.
 *
 * @param file Output filename for assembly code
 * @return Newly allocated StackContext or NULL on failure
 *
 * @note The caller is responsible for freeing the context with
 *       freeCodegenContext(). Reports specific error codes for
 *       different failure conditions.
 */
StackContext createCodeGenContext(const char *file, const char *sourceFile,
                                  const char *filename,
                                  SymbolTable symbolTable) {
  if (file == NULL) {
    repError(ERROR_INTERNAL_CODE_GENERATOR_ERROR,
             "No output filename provided");
    return NULL;
  }

  StackContext context = malloc(sizeof(struct StackContext));
  if (context == NULL) {
    repError(ERROR_MEMORY_ALLOCATION_FAILED,
             "Failed to allocate codegen context");
    return NULL;
  }
  context->file = fopen(file, "w");
  if (context->file == NULL) {
    free(context);
    repError(ERROR_INTERNAL_CODE_GENERATOR_ERROR, "Failed to open output file");
    return NULL;
  }
  context->sourceFile = sourceFile;
  context->filename = filename;
  context->symbolTable = symbolTable;
  context->variable = NULL;
  context->string = NULL;
  context->currentOffset = 0;
  context->labelCount = 1;
  context->tempCount = 1;
  context->stringCount = 0;
  return context;
}

/**
 * @brief Frees a code generation context and all associated resources.
 *
 * Performs complete cleanup of the code generation context including:
 * - All allocated stack variables
 * - Complete string table with all entries
 * - Output file closure
 * - Context structure deallocation
 *
 * @param context Code generation context to free (can be NULL)
 *
 * @note Safe to call on NULL pointer. Ensures no memory leaks by
 *       traversing and freeing all linked data structures.
 */
void freeCodegenContext(StackContext context) {
  if (context == NULL)
    return;

  StackVariable current = context->variable;
  while (current != NULL) {
    StackVariable next = current->next;
    free(current);
    current = next;
  }

  StringEntry currentStr = context->string;
  while (currentStr != NULL) {
    StringEntry next = currentStr->next;
    if (currentStr->value)
      free(currentStr->value);
    if (currentStr->label)
      free(currentStr->label);
    free(currentStr);
    currentStr = next;
  }

  if (context->file) {
    fclose(context->file);
  }

  free(context);
}

/**
 * @brief Generates assembly code to load a variable from stack into register.
 *
 * Emits assembly instructions to load a variable's value from its stack
 * location into the specified register. Handles type-specific loading
 * for both floating-point and integer/pointer types with appropriate
 * instruction selection and register naming.
 *
 * @param context Code generation context
 * @param varName Name of variable to load
 * @param reg Target register for the loaded value
 *
 * @note Reports ERROR_UNDEFINED_VARIABLE if variable not found.
 *       Uses movsd for float types and movq for all other types.
 */
void generateLoadVariable(StackContext context, const char *start, size_t len,
                          RegisterId reg, ASTNode node) {
  StackVariable var = findStackVariable(context, start, len);
  char *tempName = extractText(start, len);
  if (var == NULL) {
    reportError(ERROR_UNDEFINED_VARIABLE,
                createErrorContextFromCodeGen(node, context), tempName);
    return;
  }

  if (var->dataType == TYPE_FLOAT) {
    const char *regName = getFloatRegisterName(reg);
    fprintf(context->file, ASM_TEMPLATE_MOVSD_MEM_REG, var->stackOffset,
            regName, tempName);
  } else {
    const char *regName = getRegisterName(reg, var->dataType);
    fprintf(context->file, ASM_TEMPLATE_MOVQ_MEM_REG, var->stackOffset, regName,
            tempName);
  }
  free(tempName);
}

/**
 * @brief Generates assembly code to store register value to stack variable.
 *
 * Emits assembly instructions to store a register's value to a variable's
 * stack location. Handles type-specific storing for both floating-point
 * and integer/pointer types with appropriate instruction selection.
 *
 * @param context Code generation context
 * @param varName Name of variable to store to
 * @param reg Source register containing the value to store
 *
 * @note Reports ERROR_UNDEFINED_VARIABLE if variable not found.
 *       Uses movsd for float types and movq for all other types.
 */
void generateStoreVariable(StackContext context, const char *start, size_t len,
                           RegisterId reg, ASTNode node) {
  StackVariable var = findStackVariable(context, start, len);
  char *tempName = extractText(start, len);
  if (var == NULL) {
    reportError(ERROR_UNDEFINED_VARIABLE,
                createErrorContextFromCodeGen(node, context), tempName);
    return;
  }

  if (var->dataType == TYPE_FLOAT) {
    const char *regName = getFloatRegisterName(reg);
    fprintf(context->file, ASM_TEMPLATE_MOVSD_REG_MEM, regName,
            var->stackOffset, tempName);
  } else {
    const char *regName = getRegisterName(reg, var->dataType);
    fprintf(context->file, ASM_TEMPLATE_MOVQ_REG_MEM, regName, var->stackOffset,
            tempName);
  }
  free(tempName);
}

/**
 * @brief Generates assembly code to load a floating-point immediate value.
 *
 * Creates a floating-point constant in the .rodata section and generates
 * code to load it into an XMM register. Handles proper section switching
 * and label generation for the constant data.
 *
 * @param context Code generation context
 * @param value String representation of the float value
 * @param reg Target XMM register for the loaded value
 *
 * @note Uses .double directive for 64-bit floating-point constants.
 *       Temporarily switches to .rodata section for constant definition.
 */
void generateFloatLoadImmediate(StackContext context, const char *value,
                                RegisterId reg) {
  const char *regName = getFloatRegisterName(reg);

  char tempLabel[64];
  snprintf(tempLabel, sizeof(tempLabel), "%s%d", ASM_LABEL_PREFIX_FLOAT,
           context->tempCount++);

  ASM_EMIT_SECTION(context->file, ASM_SECTION_RODATA);
  fprintf(context->file, ASM_TEMPLATE_FLOAT_LABEL, tempLabel);
  fprintf(context->file, ASM_TEMPLATE_FLOAT_DATA, value);
  ASM_EMIT_SECTION(context->file, ASM_SECTION_TEXT);

  fprintf(context->file, ASM_TEMPLATE_MOVSD_LABEL_REG, tempLabel, regName,
          value);
}

/**
 * @brief Generates assembly code to load a string literal address.
 *
 * Adds the string to the string table (if not already present) and
 * generates code to load the string's address into a register using
 * RIP-relative addressing for position-independent code.
 *
 * @param context Code generation context
 * @param value String literal value (including quotes)
 * @param reg Target register for the string address
 *
 * @note String literals are deduplicated in the string table.
 *       Uses leaq instruction for address calculation.
 */
void generateStringLoadImmediate(StackContext context, const char *value,
                                 RegisterId reg) {
  StringEntry entry = addStringLiteral(context, value);
  if (entry == NULL)
    return;

  const char *regName = getRegisterName(reg, TYPE_STRING);
  fprintf(context->file, ASM_TEMPLATE_LEAQ_LABEL_REG, entry->label, regName,
          value);
}

/**
 * @brief Generates assembly code to load immediate values of any type.
 *
 * Main dispatch function for loading immediate values that handles
 * type-specific loading strategies. Supports all primitive types
 * with appropriate instruction selection and value formatting.
 *
 * @param context Code generation context
 * @param value String representation of the immediate value
 * @param type Data type of the immediate value
 * @param reg Target register for the loaded value
 *
 * @note Boolean values are converted to 1 (true) or 0 (false).
 *       Delegates to specialized functions for complex types.
 */
void generateLoadImmediate(StackContext context, const char *value,
                           DataType type, RegisterId reg) {
  switch (type) {
  case TYPE_FLOAT:
    generateFloatLoadImmediate(context, value, reg);
    break;

  case TYPE_STRING:
    generateStringLoadImmediate(context, value, reg);
    break;

  case TYPE_BOOL: {
    int boolVal =
        strcmp(value, "true") == 0 ? ASM_BOOL_TRUE_VALUE : ASM_BOOL_FALSE_VALUE;
    const char *regName = getRegisterName(reg, type);
    ASM_EMIT_MOVQ_IMM(context->file, boolVal == 1 ? "1" : "0", regName, "Bool",
                      value);
    break;
  }

  case TYPE_INT: {
    const char *regName = getRegisterName(reg, type);
    ASM_EMIT_MOVQ_IMM(context->file, value, regName, "Int immediate", value);
    break;
  }

  default:
    break;
  }
}

/**
 * @brief Generates floating-point binary operations using SSE instructions.
 *
 * Emits SSE2 instructions for floating-point arithmetic and comparison
 * operations. Handles proper register management and instruction selection
 * for scalar double-precision operations. Comparison operations produce
 * integer results in general-purpose registers.
 *
 * @param context Code generation context
 * @param opType Binary operation type (ADD_OP, SUB_OP, etc.)
 * @param leftReg Register containing left operand
 * @param rightReg Register containing right operand
 * @param resultReg Register for operation result
 *
 * @note Comparison operations use ucomisd followed by conditional set
 *       instructions to produce boolean results in integer registers.
 */
void generateFloatBinaryOp(StackContext context, NodeTypes opType,
                           RegisterId leftReg, RegisterId rightReg,
                           RegisterId resultReg) {
  const char *left = getFloatRegisterName(leftReg);
  const char *right = getFloatRegisterName(rightReg);
  const char *result = getFloatRegisterName(resultReg);

  // Move left operand to result register if different
  if (leftReg != resultReg) {
    fprintf(context->file, ASM_TEMPLATE_MOVSD_REG_REG, left, result);
  }

  switch (opType) {
  case ADD_OP:
    fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_ADDSD, right, result);
    break;
  case SUB_OP:
    fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_SUBSD, right, result);
    break;
  case MUL_OP:
    fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_MULSD, right, result);
    break;
  case DIV_OP:
    fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_DIVSD, right, result);
    break;
  case EQUAL_OP:
  case NOT_EQUAL_OP:
  case LESS_THAN_OP:
  case GREATER_THAN_OP:
  case LESS_EQUAL_OP:
  case GREATER_EQUAL_OP: {
    // Float comparison - result goes to integer register
    const char *intResult = getRegisterName(resultReg, TYPE_INT);
    fprintf(context->file, ASM_TEMPLATE_FLOAT_CMP, right, result);

    const char *setInstruction;
    switch (opType) {
    case EQUAL_OP:
      setInstruction = ASM_SETE;
      break;
    case NOT_EQUAL_OP:
      setInstruction = ASM_SETNE;
      break;
    case LESS_THAN_OP:
      setInstruction = ASM_SETB;
      break;
    case GREATER_THAN_OP:
      setInstruction = ASM_SETA;
      break;
    case LESS_EQUAL_OP:
      setInstruction = ASM_SETBE;
      break;
    case GREATER_EQUAL_OP:
      setInstruction = ASM_SETAE;
      break;
    default:
      setInstruction = ASM_SETE;
      break;
    }

    fprintf(context->file, ASM_TEMPLATE_CMP_SET, setInstruction);
    fprintf(context->file, ASM_TEMPLATE_CMP_EXTEND, intResult);
    break;
  }
  default:
    emitComment(context, "Unknown float binary operation");
    break;
  }
}

/**
 * @brief Generates floating-point unary operations using SSE instructions.
 *
 * Emits SSE2 instructions for floating-point unary operations including
 * negation and positive (no-op). Handles proper register management and
 * uses bit manipulation for floating-point negation.
 *
 * @param context Code generation context
 * @param opType Unary operation type
 * @param operandReg Register containing operand
 * @param resultReg Register for operation result
 *
 * @note Floating-point negation uses XOR with sign bit mask rather
 *       than arithmetic negation for IEEE 754 compliance.
 */
void generateFloatUnaryOp(StackContext context, NodeTypes opType,
                          RegisterId operandReg, RegisterId resultReg) {
  const char *operand = getFloatRegisterName(operandReg);
  const char *result = getFloatRegisterName(resultReg);

  if (operandReg != resultReg) {
    fprintf(context->file, ASM_TEMPLATE_MOVSD_REG_REG, operand, result);
  }

  switch (opType) {
  case UNARY_MINUS_OP: {
    char negLabel[64];
    snprintf(negLabel, sizeof(negLabel), "%s%d", ASM_LABEL_PREFIX_FLOAT_NEG,
             context->tempCount++);

    fprintf(context->file, ASM_SECTION_RODATA);
    fprintf(context->file, ASM_TEMPLATE_FLOAT_LABEL, negLabel);
    fprintf(context->file, ASM_TEMPLATE_FLOAT_MASK, ASM_FLOAT_SIGN_MASK);
    fprintf(context->file, "%s\n", ASM_SECTION_TEXT);

    fprintf(context->file, ASM_TEMPLATE_XOR_RIP, ASM_XORPD, negLabel, result);
    break;
  }
  case UNARY_PLUS_OP:
    break;
  default:
    emitComment(context, "Unknown float unary operation");
    break;
  }
}

/**
 * @brief Generates string operations (primarily comparison).
 *
 * Handles string operations which are currently limited to pointer
 * comparison for equality and inequality. Emits assembly code for
 * comparing string addresses and setting boolean results.
 *
 * @param context Code generation context
 * @param opType String operation type
 * @param leftReg Register containing left string pointer
 * @param rightReg Register containing right string pointer
 * @param resultReg Register for boolean result
 *
 * @note Currently implements simplified pointer comparison.
 *       Future enhancements could add proper string comparison
 *       using strcmp or inline character-by-character comparison.
 */
void generateStringOperation(StackContext context, NodeTypes opType,
                             RegisterId leftReg, RegisterId rightReg,
                             RegisterId resultReg) {
  const char *left = getRegisterName(leftReg, TYPE_STRING);
  const char *right = getRegisterName(rightReg, TYPE_STRING);
  const char *result = getRegisterName(resultReg, TYPE_INT);

  switch (opType) {
  case EQUAL_OP:
  case NOT_EQUAL_OP: {
    emitComment(context, "String comparison (simplified - pointer comparison)");
    fprintf(context->file, ASM_TEMPLATE_CMP_REGS, right, left);

    const char *setInstruction = (opType == EQUAL_OP) ? ASM_SETE : ASM_SETNE;
    fprintf(context->file, ASM_TEMPLATE_CMP_SET, setInstruction);
    fprintf(context->file, ASM_TEMPLATE_CMP_EXTEND, result);
    break;
  }
  default:
    emitComment(context, "Unsupported string operation");
    break;
  }
}

/**
 * @brief Generates binary operations with type-aware instruction selection.
 *
 * Main dispatch function for binary operations that handles all data types
 * and delegates to appropriate specialized functions. Manages register
 * allocation and ensures proper instruction selection based on operand types.
 * Includes optimization for operand ordering when literals are involved.
 *
 * @param context Code generation context
 * @param opType Binary operation type
 * @param leftReg Register containing left operand
 * @param rightReg Register containing right operand
 * @param resultReg Register for operation result
 * @param operandType Data type of the operands
 * @param invert Flag indicating if operands needs to be inverted for "a - b =
 * -(b - a)" math properties
 *
 * @note Delegates to specialized functions for floating-point and string
 *       operations while handling integer/boolean operations directly.
 *       The invert flag triggers result negation for certain operations
 *       when operand order was optimized during expression generation.
 */
void generateBinaryOp(StackContext context, NodeTypes opType,
                      RegisterId leftReg, RegisterId rightReg,
                      RegisterId resultReg, DataType operandType, int invert) {
  if (operandType == TYPE_FLOAT) {
    generateFloatBinaryOp(context, opType, leftReg, rightReg, resultReg);
    return;
  }

  if (operandType == TYPE_STRING) {
    generateStringOperation(context, opType, leftReg, rightReg, resultReg);
    return;
  }

  // Integer/boolean operations
  const char *left = getRegisterName(leftReg, operandType);
  const char *right = getRegisterName(rightReg, operandType);
  const char *result = getRegisterName(resultReg, operandType);

  // Handle the case where we might overwrite an operand
  if (rightReg == resultReg && leftReg != resultReg) {
    // Right operand is in result register, will be overwritten if we move left
    // to result
    switch (opType) {
    case ADD_OP:
      // Addition is commutative: instead of result = left + right, do result =
      // result + left
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_ADDQ, left, result);
      break;
    case MUL_OP:
      // Multiplication is commutative
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_IMULQ, left, result);
      break;
    case SUB_OP:
      // Subtraction is NOT commutative, need to save right first
      fprintf(context->file, "    movq %s, %%r11    # Save right operand\n",
              right);
      fprintf(context->file, ASM_TEMPLATE_MOVQ_REG_REG, left, result);
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_SUBQ, "%r11", result);
      break;
    case DIV_OP:
    case MOD_OP:
      // Division/modulo need special handling with RAX/RDX
      fprintf(context->file, "    movq %s, %%r11    # Save right operand\n",
              right);
      fprintf(context->file, ASM_TEMPLATE_MOVQ_REG_REG, left, "%rax");
      fprintf(context->file, ASM_TEMPLATE_DIV_SETUP, "%rax", "%r11");
      if (opType == DIV_OP && resultReg != REG_RAX) {
        fprintf(context->file, ASM_TEMPLATE_DIV_RESULT_QUOT, result);
      } else if (opType == MOD_OP && resultReg != REG_RDX) {
        fprintf(context->file, ASM_TEMPLATE_DIV_RESULT_REM, result);
      }
      break;
    case LOGIC_AND:
      // AND is commutative
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_ANDQ, left, result);
      break;
    case LOGIC_OR:
      // OR is commutative
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_ORQ, left, result);
      break;
    case EQUAL_OP:
    case NOT_EQUAL_OP:
    case LESS_THAN_OP:
    case GREATER_THAN_OP:
    case LESS_EQUAL_OP:
    case GREATER_EQUAL_OP: {
      // Comparisons need both operands, save right first
      fprintf(context->file, "    movq %s, %%r11    # Save right operand\n",
              right);
      fprintf(context->file, ASM_TEMPLATE_MOVQ_REG_REG, left, result);
      fprintf(context->file, ASM_TEMPLATE_CMP_REGS, "%r11", result);

      const char *setInstruction;
      switch (opType) {
      case EQUAL_OP:
        setInstruction = ASM_SETE;
        break;
      case NOT_EQUAL_OP:
        setInstruction = ASM_SETNE;
        break;
      case LESS_THAN_OP:
        setInstruction = ASM_SETL;
        break;
      case GREATER_THAN_OP:
        setInstruction = ASM_SETG;
        break;
      case LESS_EQUAL_OP:
        setInstruction = ASM_SETLE;
        break;
      case GREATER_EQUAL_OP:
        setInstruction = ASM_SETGE;
        break;
      default:
        setInstruction = ASM_SETE;
        break;
      }
      fprintf(context->file, ASM_TEMPLATE_CMP_SET, setInstruction);
      fprintf(context->file, ASM_TEMPLATE_CMP_EXTEND, result);
      break;
    }
    default:
      emitComment(context, "Unknown binary operation");
      break;
    }
  } else {
    // Normal case: left and result are different, or left is already in result
    if (leftReg != resultReg) {
      fprintf(context->file, ASM_TEMPLATE_MOVQ_REG_REG, left, result);
    }

    switch (opType) {
    case ADD_OP:
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_ADDQ, right, result);
      break;
    case SUB_OP:
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_SUBQ, right, result);
      break;
    case MUL_OP:
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_IMULQ, right, result);
      break;
    case DIV_OP:
      fprintf(context->file, ASM_TEMPLATE_DIV_SETUP, result, right);
      if (resultReg != REG_RAX) {
        fprintf(context->file, ASM_TEMPLATE_DIV_RESULT_QUOT, result);
      }
      break;
    case MOD_OP:
      fprintf(context->file, ASM_TEMPLATE_DIV_SETUP, result, right);
      if (resultReg != REG_RDX) {
        fprintf(context->file, ASM_TEMPLATE_DIV_RESULT_REM, result);
      }
      break;
    case EQUAL_OP:
    case NOT_EQUAL_OP:
    case LESS_THAN_OP:
    case GREATER_THAN_OP:
    case LESS_EQUAL_OP:
    case GREATER_EQUAL_OP: {
      const char *setInstruction;
      switch (opType) {
      case EQUAL_OP:
        setInstruction = ASM_SETE;
        break;
      case NOT_EQUAL_OP:
        setInstruction = ASM_SETNE;
        break;
      case LESS_THAN_OP:
        setInstruction = ASM_SETL;
        break;
      case GREATER_THAN_OP:
        setInstruction = ASM_SETG;
        break;
      case LESS_EQUAL_OP:
        setInstruction = ASM_SETLE;
        break;
      case GREATER_EQUAL_OP:
        setInstruction = ASM_SETGE;
        break;
      default:
        setInstruction = ASM_SETE;
        break;
      }
      fprintf(context->file, ASM_TEMPLATE_CMP_REGS, right, result);
      fprintf(context->file, ASM_TEMPLATE_CMP_SET, setInstruction);
      fprintf(context->file, ASM_TEMPLATE_CMP_EXTEND, result);
      break;
    }
    case LOGIC_AND:
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_ANDQ, right, result);
      break;
    case LOGIC_OR:
      fprintf(context->file, ASM_TEMPLATE_BINARY_OP, ASM_ORQ, right, result);
      break;
    default:
      emitComment(context, "Unknown binary operation");
      break;
    }
  }

  if (invert == 1) {
    emitComment(context, "Invert result a - b = -(b - a)");
    fprintf(context->file, "    #%s = -%s\n" ASM_TEMPLATE_UNARY_OP, result,
            result, ASM_NEGQ, result);
  }
}

/**
 * @brief Generates code to evaluate an expression and store result in register.
 *
 * Recursively processes expression AST nodes and generates assembly code
 * to evaluate the expression, storing the final result in the specified
 * register. Handles all expression types including literals, variables,
 * binary operations, unary operations, and increment/decrement operations.
 *
 * Key features:
 * - Automatic register selection based on data types
 * - Recursive expression evaluation with proper precedence
 * - Type-aware instruction generation
 * - Postfix increment/decrement handling with value preservation
 * - Comprehensive error reporting for malformed expressions
 *
 * @param node Expression AST node to evaluate
 * @param context Code generation context
 * @param preferredReg Preferred output register (may be changed for type
 * compatibility)
 * @return RegisterId containing the expression result
 *
 * @note Float expressions automatically use XMM registers regardless of
 *       the preferred register. Postfix operations preserve the original
 *       value while updating the variable.
 */
RegisterId generateExpressionToRegister(ASTNode node, StackContext context,
                                        RegisterId preferredReg) {
  if (node == NULL)
    return preferredReg;

  switch (node->nodeType) {
  case INT_LIT:
  case FLOAT_LIT:
  case BOOL_LIT: {
    DataType type = getDataTypeFromNode(node->nodeType);
    if (type == TYPE_FLOAT) {
      preferredReg = REG_XMM0; // Use XMM register for floats
    }
    char *tempVal = extractText(node->start, node->length);
    generateLoadImmediate(context, tempVal, type, preferredReg);
    if (tempVal)
      free(tempVal);
    return preferredReg;
  }

  case STRING_LIT: {
    char *tempVal = extractText(node->start, node->length);
    generateLoadImmediate(context, tempVal, TYPE_STRING, preferredReg);
    if (tempVal)
      free(tempVal);
    return preferredReg;
  }

  case VARIABLE: {
    StackVariable var = findStackVariable(context, node->start, node->length);
    if (var && var->dataType == TYPE_FLOAT) {
      preferredReg = REG_XMM0; // Use XMM register for float variables
    }
    generateLoadVariable(context, node->start, node->length, preferredReg,
                         node);
    return preferredReg;
  }

  case ADD_OP:
  case SUB_OP:
  case MUL_OP:
  case DIV_OP:
  case MOD_OP:
  case EQUAL_OP:
  case NOT_EQUAL_OP:
  case LESS_THAN_OP:
  case GREATER_THAN_OP:
  case LESS_EQUAL_OP:
  case GREATER_EQUAL_OP:
  case LOGIC_AND:
  case LOGIC_OR: {
    if (node->children == NULL || node->children->brothers == NULL) {
      repError(ERROR_INTERNAL_PARSER_ERROR,
               "Binary operation missing operands");
      return preferredReg;
    }

    DataType operandType = getOperandType(node->children, context);
    if (operandType == TYPE_UNKNOWN) {
      operandType = getOperandType(node->children->brothers, context);
    }

    RegisterId leftReg, rightReg;
    ASTNode left;
    ASTNode right;
    int invert = 0;
    if (isLiteral(node->children) && !isLiteral(node->children->brothers) &&
        node->nodeType == SUB_OP) {
      left = node->children->brothers;
      right = node->children;
      invert = 1;
    } else {
      left = node->children;
      right = node->children->brothers;
    }
    int needSpill = !isLiteral(left) || !isLiteral(right);
    if (operandType == TYPE_FLOAT) {
      leftReg = REG_XMM0;
      rightReg = REG_XMM1;
    } else {
      leftReg = REG_RAX;
      rightReg = REG_RBX;
    }
    leftReg = generateExpressionToRegister(left, context, leftReg);
    if (needSpill) {
      spillRegisterToStack(context, leftReg, operandType);
      rightReg = generateExpressionToRegister(right, context, REG_RAX);
      leftReg = getOppositeBranchRegister(rightReg);
      restoreRegisterFromStack(context, leftReg, operandType);
    } else {
      rightReg = generateExpressionToRegister(right, context, rightReg);
    }
    RegisterId resultReg = preferredReg;
    if (operandType == TYPE_FLOAT && preferredReg < REG_XMM0) {
      resultReg = REG_XMM0;
    } else if (operandType != TYPE_FLOAT && preferredReg >= REG_XMM0) {
      resultReg = REG_RAX;
    }

    generateBinaryOp(context, node->nodeType, leftReg, rightReg, resultReg,
                     operandType, invert);
    return preferredReg;
  }

  case UNARY_MINUS_OP:
  case UNARY_PLUS_OP:
  case LOGIC_NOT:
  case PRE_INCREMENT:
  case PRE_DECREMENT: {
    if (node->children == NULL) {
      repError(ERROR_INTERNAL_PARSER_ERROR, "Unary operation missing operand");
      return preferredReg;
    }

    DataType operandType = getOperandType(node->children, context);
    if (operandType == TYPE_FLOAT) {
      preferredReg = REG_XMM0;
    }

    RegisterId operandReg =
        generateExpressionToRegister(node->children, context, preferredReg);
    generateUnaryOp(context, node->nodeType, operandReg, preferredReg,
                    operandType);
    return preferredReg;
  }

  case POST_INCREMENT:
  case POST_DECREMENT: {
    if (node->children == NULL || node->children->nodeType != VARIABLE) {
      repError(ERROR_INTERNAL_PARSER_ERROR,
               "Postfix operation requires variable");
      return preferredReg;
    }

    DataType operandType = getOperandType(node->children, context);
    if (operandType == TYPE_FLOAT) {
      preferredReg = REG_XMM0;
    }

    // Load current value
    generateLoadVariable(context, node->children->start, node->children->length,
                         preferredReg, node);

    // Store incremented/decremented value back to variable
    RegisterId tempReg = (operandType == TYPE_FLOAT) ? REG_XMM1
                         : (preferredReg == REG_RAX) ? REG_RBX
                                                     : REG_RAX;
    generateLoadVariable(context, node->children->start, node->children->length,
                         tempReg, node);
    generateUnaryOp(context, node->nodeType, tempReg, tempReg, operandType);
    generateStoreVariable(context, node->children->start,
                          node->children->length, tempReg, node);

    return preferredReg; // Return original value
  }
  case FUNCTION_CALL: {
    if (isBuiltinFunction(node->start, node->length)) {
      generateBuiltinFunctionCall(node, context);
      return preferredReg; // Most built-ins don't return values
    }
    // User-defined function call
    emitComment(context, "Call user function");

    // Generate arguments in calling convention registers
    ASTNode argList = node->children;
    if (argList && argList->nodeType == ARGUMENT_LIST) {
      ASTNode arg = argList->children;
      RegisterId argRegs[] = {REG_RDI, REG_RSI, REG_RDX,
                              REG_RCX, REG_R8,  REG_R9};
      int argIndex = 0;

      while (arg != NULL && argIndex < 6) {
        generateExpressionToRegister(arg, context, argRegs[argIndex]);
        arg = arg->brothers;
        argIndex++;
      }
    }
    char *tempVal = extractText(node->start, node->length);
    // Call the function
    fprintf(context->file, "    call %s\n", tempVal);
    if (tempVal)
      free(tempVal);

    // Return value is in RAX
    return REG_RAX;
  }
  case MEMBER_ACCESS: {
    if (node->children == NULL || node->children->brothers == NULL) {
      repError(ERROR_INTERNAL_PARSER_ERROR, "Member access missing operands");
      return preferredReg;
    }

    ASTNode objectNode = node->children;
    ASTNode fieldNode = objectNode->brothers;

    if (objectNode->nodeType != VARIABLE) {
      repError(ERROR_INVALID_OPERATION_FOR_TYPE,
               "Member access requires variable");
      return preferredReg;
    }

    StackVariable structVar =
        findStackVariable(context, objectNode->start, objectNode->length);
    if (!structVar || structVar->dataType != TYPE_STRUCT) {
      repError(ERROR_INVALID_OPERATION_FOR_TYPE, "Member access on non-struct");
      return preferredReg;
    }

    StructField field = findStructField(structVar->structType, fieldNode->start,
                                        fieldNode->length);
    if (!field) {
      repError(ERROR_UNDEFINED_VARIABLE, "Struct field not found");
      return preferredReg;
    }

    int memberOffset = structVar->stackOffset + field->offset;

    if (field->type == TYPE_FLOAT) {
      preferredReg = REG_XMM0;
      fprintf(context->file,
              "    movsd %d(%%rbp), %s    # Load struct member\n", memberOffset,
              getFloatRegisterName(preferredReg));
    } else {
      fprintf(context->file,
              "    movq %d(%%rbp), %s     # Load struct member\n", memberOffset,
              getRegisterName(preferredReg, field->type));
    }

    return preferredReg;
  }

  default:
    emitComment(context, "Unknown expression type");
    return preferredReg;
  }
}

/**
 * @brief Generates assembly code for conditional statements (if-else).
 *
 * Implements conditional control flow using labels and conditional jumps.
 * Handles both simple conditionals (if-only) and full if-else constructs
 * with proper label generation and jump optimization.
 *
 * Process:
 * 1. Generate unique labels for else and end points
 * 2. Evaluate condition expression
 * 3. Test condition and jump to appropriate branch
 * 4. Generate true branch code
 * 5. Generate false branch code (if present)
 * 6. Emit end label for control flow continuation
 *
 * @param node IF_CONDITIONAL AST node
 * @param context Code generation context
 * @return 1 on success, 0 on failure
 *
 * @note Uses testq instruction for boolean evaluation and conditional
 *       jumps (jz) for branch selection. Optimizes single-branch
 *       conditionals by skipping unnecessary labels.
 */
int generateConditional(ASTNode node, StackContext context) {
  if (node->children == NULL)
    return 0;

  char elseLabel[64], endLabel[64];
  generateLabel(context, ASM_LABEL_PREFIX_ELSE, elseLabel, sizeof(elseLabel));
  generateLabel(context, ASM_LABEL_PREFIX_END_IF, endLabel, sizeof(endLabel));

  // Generate condition
  emitComment(context, "Evaluate condition");
  RegisterId condReg =
      generateExpressionToRegister(node->children, context, REG_RAX);

  // Test condition and jump to else if false
  fprintf(context->file, ASM_TEMPLATE_TEST_REG,
          getRegisterName(condReg, TYPE_INT),
          getRegisterName(condReg, TYPE_INT));

  ASTNode trueBranch = node->children->brothers;
  ASTNode falseBranch = trueBranch ? trueBranch->brothers : NULL;

  if (falseBranch) {
    fprintf(context->file, ASM_TEMPLATE_CONDITIONAL_JUMP, ASM_JZ, elseLabel);
  } else {
    fprintf(context->file, ASM_TEMPLATE_CONDITIONAL_JUMP, ASM_JZ, endLabel);
  }

  // Generate true branch
  if (trueBranch && trueBranch->nodeType == IF_TRUE_BRANCH &&
      trueBranch->children) {
    emitComment(context, "True branch");
    generateNodeCode(trueBranch->children, context);
  }

  // Generate false branch if it exists
  if (falseBranch) {
    fprintf(context->file, ASM_TEMPLATE_JUMP, endLabel);
    fprintf(context->file, ASM_TEMPLATE_LABEL, elseLabel);

    if (falseBranch->nodeType == ELSE_BRANCH && falseBranch->children) {
      emitComment(context, "False branch");
      generateNodeCode(falseBranch->children, context);
    }
  }

  fprintf(context->file, ASM_TEMPLATE_LABEL, endLabel);
  return 1;
}

/**
 * @brief Generates assembly code for loop statements (while loops).
 *
 * Implements loop control flow using labels and conditional jumps.
 * Creates a loop structure with condition evaluation at the beginning
 * and proper exit handling.
 *
 * Process:
 * 1. Generate unique labels for loop start and end
 * 2. Emit loop start label
 * 3. Evaluate loop condition
 * 4. Test condition and exit if false
 * 5. Generate loop body code
 * 6. Jump back to condition evaluation
 * 7. Emit end label for loop exit
 *
 * @param node LOOP_STATEMENT AST node
 * @param context Code generation context
 * @return 1 on success, 0 on failure
 *
 * @note Uses testq instruction for condition evaluation and implements
 *       a standard while-loop pattern with condition-controlled exit.
 */
int generateLoop(ASTNode node, StackContext context) {
  if (node->children == NULL || node->children->brothers == NULL)
    return 0;

  char loopLabel[64], endLabel[64];
  generateLabel(context, ASM_LABEL_PREFIX_LOOP, loopLabel, sizeof(loopLabel));
  generateLabel(context, ASM_LABEL_PREFIX_END_LOOP, endLabel, sizeof(endLabel));

  ASTNode condition = node->children;
  ASTNode body = node->children->brothers;

  fprintf(context->file, ASM_TEMPLATE_LABEL, loopLabel);

  // Generate condition
  emitComment(context, "Loop condition");
  RegisterId condReg =
      generateExpressionToRegister(condition, context, REG_RAX);

  // Test condition and exit if false
  fprintf(context->file, ASM_TEMPLATE_TEST_REG,
          getRegisterName(condReg, TYPE_INT),
          getRegisterName(condReg, TYPE_INT));
  fprintf(context->file, ASM_TEMPLATE_CONDITIONAL_JUMP, ASM_JZ, endLabel);

  // Generate loop body
  emitComment(context, "Loop body");
  generateNodeCode(body, context);

  // Jump back to condition
  fprintf(context->file, ASM_TEMPLATE_JUMP, loopLabel);
  fprintf(context->file, ASM_TEMPLATE_LABEL, endLabel);

  return 1;
}

int generateBuiltinFunctionCall(ASTNode node, StackContext context) {
  if (node == NULL || node->start == NULL)
    return 0;

  // Get argument types for overload resolution
  ASTNode argList = node->children;
  int argCount = 0;
  DataType *argTypes = NULL;

  // Count and extract argument types (existing dynamic code)
  if (argList && argList->nodeType == ARGUMENT_LIST) {
    ASTNode arg = argList->children;
    while (arg != NULL) {
      argCount++;
      arg = arg->brothers;
    }
    if (argCount > 0) {
      argTypes = malloc(argCount * sizeof(DataType));
      if (argTypes == NULL) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED,
                 "Failed to allocate argument types");
        return 0;
      }

      arg = argList->children;
      for (int i = 0; i < argCount && arg != NULL; i++) {
        argTypes[i] = getOperandType(arg, context);
        arg = arg->brothers;
      }
    }
  }

  BuiltInId builtinId =
      resolveOverload(node->start, node->length, argTypes, argCount);
  if (argTypes != NULL) {
    free(argTypes);
  }
  switch (builtinId) {
  case BUILTIN_PRINT_STRING: {
    emitComment(context, "print(string)");

    if (argList && argList->children) {
      RegisterId strReg =
          generateExpressionToRegister(argList->children, context, REG_RDI);

      fprintf(context->file,
              "    movq %s, %%rdi       # String pointer\n"
              "    call print_str_z     # Runtime calculates length & prints\n",
              getRegisterName(strReg, TYPE_STRING));
    }
    break;
  }

  case BUILTIN_PRINT_INT: {
    emitComment(context, "print(int)");

    if (argList && argList->children) {
      RegisterId intReg =
          generateExpressionToRegister(argList->children, context, REG_RDI);

      fprintf(context->file,
              "    movq %s, %%rdi       # Integer value\n"
              "    call print_int       # Runtime converts & prints\n",
              getRegisterName(intReg, TYPE_INT));
    }
    break;
  }

  case BUILTIN_PRINT_BOOL: {
    emitComment(context, "print(bool)");

    if (argList && argList->children) {
      RegisterId boolReg =
          generateExpressionToRegister(argList->children, context, REG_RDI);

      fprintf(context->file,
              "    movq %s, %%rdi       # Boolean value\n"
              "    call print_bool      # Runtime prints 'true'/'false'\n",
              getRegisterName(boolReg, TYPE_BOOL));
    }
    break;
  }

  case BUILTIN_PRINT_FLOAT: {
    emitComment(context,
                "print(float) - TODO: implement float conversion in runtime");

    if (argList && argList->children) {
      RegisterId floatReg =
          generateExpressionToRegister(argList->children, context, REG_XMM0);

      fprintf(context->file,
              "    cvttsd2si %s, %%rdi  # Convert float to int (simplified)\n"
              "    call print_int       # Print as integer for now\n",
              getFloatRegisterName(floatReg));
    }
    break;
  }

  case BUILTIN_EXIT: {
    emitComment(context, "exit(code)");

    if (argList && argList->children) {
      RegisterId exitReg =
          generateExpressionToRegister(argList->children, context, REG_RDI);

      fprintf(context->file,
              "    movq %s, %%rdi       # Exit status\n"
              "    call exit_program    # Runtime exits cleanly\n",
              getRegisterName(exitReg, TYPE_INT));
    }
    break;
  }

  default:
    return 0;
  }

  return 1;
}

/**
 * @brief Generates unary operations with type-aware instruction selection.
 *
 * Handles all unary operations including arithmetic unary minus/plus,
 * logical NOT, and increment/decrement operations. Delegates floating-point
 * operations to specialized functions while handling integer operations
 * directly with appropriate x86-64 instructions.
 *
 * @param context Code generation context
 * @param opType Unary operation type
 * @param operandReg Register containing operand
 * @param resultReg Register for operation result
 * @param operandType Data type of the operand
 *
 * @note Logical NOT uses test/set pattern to convert any non-zero value
 *       to 0 and zero to 1. Increment/decrement use efficient inc/dec
 *       instructions for both prefix and postfix variants.
 */
void generateUnaryOp(StackContext context, NodeTypes opType,
                     RegisterId operandReg, RegisterId resultReg,
                     DataType operandType) {
  if (operandType == TYPE_FLOAT) {
    generateFloatUnaryOp(context, opType, operandReg, resultReg);
    return;
  }

  const char *operand = getRegisterName(operandReg, operandType);
  const char *result = getRegisterName(resultReg, operandType);

  if (operandReg != resultReg) {
    fprintf(context->file, ASM_TEMPLATE_MOVQ_REG_REG, operand, result);
  }

  switch (opType) {
  case UNARY_MINUS_OP:
    fprintf(context->file, ASM_TEMPLATE_UNARY_OP, ASM_NEGQ, result);
    break;
  case UNARY_PLUS_OP:
    break;
  case LOGIC_NOT:
    fprintf(context->file, ASM_TEMPLATE_TEST_REG, result, result);
    fprintf(context->file, ASM_TEMPLATE_CMP_SET, ASM_SETZ);
    fprintf(context->file, ASM_TEMPLATE_CMP_EXTEND, result);
    break;
  case PRE_INCREMENT:
  case POST_INCREMENT:
    fprintf(context->file, ASM_TEMPLATE_UNARY_OP, ASM_INCQ, result);
    break;
  case PRE_DECREMENT:
  case POST_DECREMENT:
    fprintf(context->file, ASM_TEMPLATE_UNARY_OP, ASM_DECQ, result);
    break;
  default:
    emitComment(context, "Unknown unary operation");
    break;
  }
}

/**
 * @brief Main entry point for code generation from AST to assembly file.
 *
 * Orchestrates the complete code generation process from Abstract Syntax Tree
 * to executable x86-64 assembly code. Manages the entire generation pipeline
 * including context creation, code emission, and resource cleanup.
 *
 * Process:
 * 1. Validate input parameters
 * 2. Create code generation context
 * 3. Emit assembly preamble (string table, program setup)
 * 4. Generate code for entire AST
 * 5. Emit assembly epilogue (program exit)
 * 6. Clean up resources and close output file
 *
 * @param ast Root AST node (typically PROGRAM node)
 * @param outputFile Filename for generated assembly code
 * @return 1 on successful code generation, 0 on failure
 *
 * @note Integrates with the global error reporting system and provides
 *       comprehensive error handling for all failure modes. The generated
 *       assembly is ready for linking with the GNU assembler and linker.
 */
int generateCode(ASTNode ast, const char *outputFile, const char *sourceCode,
                 const char *filename, SymbolTable symbolTable) {
  // Validate input parameters
  if (ast == NULL) {
    repError(ERROR_INTERNAL_PARSER_ERROR, "Cannot generate code from null AST");
    return 0;
  }

  if (outputFile == NULL) {
    repError(ERROR_INTERNAL_CODE_GENERATOR_ERROR,
             "No output filename provided");
    return 0;
  }

  // Create code generation context
  StackContext context =
      createCodeGenContext(outputFile, sourceCode, filename, symbolTable);
  if (context == NULL) {
    return 0; // Error already reported in createCodeGenContext
  }

  collectStringLiterals(ast, context);

  // Generate assembly preamble (includes string table and program setup)
  emitPreamble(context);

  // Generate code for the entire AST
  int success = generateNodeCode(ast, context);

  if (!success) {
    emitComment(context, "Code generation failed - incomplete output");
  }

  // Clean up and close output file
  freeCodegenContext(context);

  return success;
}
