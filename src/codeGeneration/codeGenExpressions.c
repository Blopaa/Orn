//
// Created by pablo on 22/09/2025.
//

#include "codeGenExpressions.h"

#include "asmTemplate.h"
#include "builtIns.h"
#include "codeGenOperations.h"

#include <stdlib.h>

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
      spillRegisterToTempVar(context, leftReg, operandType, TEMP_VAR_A);
      rightReg = generateExpressionToRegister(right, context, REG_RAX);
      leftReg = getOppositeBranchRegister(rightReg);
      restoreRegisterFromTempVar(context, leftReg, operandType, TEMP_VAR_A);
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

/**
 * @brief Generates assembly code for built-in function calls with proper
 * argument handling. Resolves function overloads based on argument types and
 * emits appropriate runtime calls for print operations, type conversions, and
 * system functions like exit.
 */
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
      if (strReg != REG_RDI) {
        fprintf(context->file, "    movq %s, %%rdi       # String pointer\n",
                getRegisterName(strReg, TYPE_STRING));
      }
      fprintf(
          context->file,
          "    call print_str_z     # Runtime calculates length & prints\n");
    }
    break;
  }

  case BUILTIN_PRINT_INT: {
    emitComment(context, "print(int)");

    if (argList && argList->children) {
      RegisterId intReg =
          generateExpressionToRegister(argList->children, context, REG_RDI);

      if (intReg != REG_RDI) {
        fprintf(context->file, "    movq %s, %%rdi       # Integer value\n",
                getRegisterName(intReg, TYPE_INT));
      }
      fprintf(context->file,
              "    call print_int       # Runtime converts & prints\n");
    }
    break;
  }

  case BUILTIN_PRINT_BOOL: {
    emitComment(context, "print(bool)");

    if (argList && argList->children) {
      RegisterId boolReg =
          generateExpressionToRegister(argList->children, context, REG_RDI);

      if (boolReg != REG_RDI) {
        fprintf(context->file, "    movq %s, %%rdi       # Boolean value\n",
                getRegisterName(boolReg, TYPE_BOOL));
      }
      fprintf(context->file,
              "    call print_bool      # Runtime prints 'true'/'false'\n");
    }
    break;
  }
  case BUILTIN_PRINT_FLOAT: {
    emitComment(context,
                "print(float) - TODO: implement float conversion in runtime");

    if (argList && argList->children) {
      RegisterId floatReg =
          generateExpressionToRegister(argList->children, context, REG_XMM0);
      if (floatReg != REG_RDI) {
        fprintf(
            context->file,
            "    cvttsd2si %s, %%rdi  # Convert float to int (simplified)\n",
            getFloatRegisterName(floatReg));
      }
      fprintf(context->file,

              "    call print_int       # Print as integer for now\n");
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