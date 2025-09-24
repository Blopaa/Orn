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
#include "codeGenExpressions.h"
#include "codeGenOperations.h"
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
      } else {
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
  context->currentOffset = 16;
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
    fprintf(context->file, "    movss %d(%%rbp), %s    # Load float %s\n",
            var->stackOffset, regName, tempName);
  } else {
    const char *regName = getRegisterNameForSize(reg, var->dataType);
    const char *suffix = getInstructionSuffix(var->dataType);

    if (var->dataType == TYPE_BOOL) {
      fprintf(context->file,
              "    movzbl %d(%%rbp), %s    # Load bool %s (zero-extended)\n",
              var->stackOffset, getRegisterNameForSize(reg, TYPE_INT),
              tempName);
    } else if (var->dataType == TYPE_INT) {
      fprintf(context->file, "    mov%s %d(%%rbp), %s    # Load int %s\n",
              suffix, var->stackOffset, regName, tempName);
    } else {
      fprintf(context->file, "    movq %d(%%rbp), %s    # Load %s\n",
              var->stackOffset, regName, tempName);
    }
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
    fprintf(context->file, "    movss %s, %d(%%rbp)    # Store float %s\n",
            regName, var->stackOffset, tempName);
  } else {
    const char *regName = getRegisterNameForSize(reg, var->dataType);
    const char *suffix = getInstructionSuffix(var->dataType);

    fprintf(context->file, "    mov%s %s, %d(%%rbp)    # Store %s\n",
            suffix, regName, var->stackOffset, tempName);
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
    const char *regName = getRegisterNameForSize(reg, TYPE_INT);
    fprintf(context->file, "    movl $%d, %s    # Load bool: %s\n", boolVal,
            regName, value);
    break;
  }

  case TYPE_INT: {
    const char *regName = getRegisterNameForSize(reg, TYPE_INT);
    fprintf(context->file, "    movl $%s, %s    # Load int: %s\n", value,
            regName, value);
    break;
  }

  default:
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
