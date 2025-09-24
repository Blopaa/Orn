#include "codeGenOperations.h"

#include "asmTemplate.h"

#include <string.h>

/**
 * @brief Generates binary operations with type-aware instruction selection and
 * register management. Handles all data types including special cases for
 * operand ordering, register conflicts, and mathematical operations requiring
 * specific x86-64 instruction sequences.
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

  const char *left = getRegisterNameForSize(leftReg, operandType);
  const char *right = getRegisterNameForSize(rightReg, operandType);
  const char *result = getRegisterNameForSize(resultReg, operandType);
  const char *suffix = getInstructionSuffix(operandType);

  switch (opType) {
  case ADD_OP:
    // Addition is commutative
    if (leftReg != resultReg && rightReg != resultReg) {
      fprintf(context->file, "    mov%s %s, %s\n", suffix, left, result);
      fprintf(context->file, "    add%s %s, %s\n", suffix, right, result);
    } else if (rightReg == resultReg) {
      // result already has right, just add left
      fprintf(context->file, "    add%s %s, %s\n", suffix, left, result);
    } else {
      // leftReg == resultReg, normal case
      fprintf(context->file, "    add%s %s, %s\n", suffix, right, result);
    }
    break;

  case SUB_OP:
    if (leftReg != resultReg) {
      fprintf(context->file, "    mov%s %s, %s\n", suffix, left, result);
    }
    fprintf(context->file, "    sub%s %s, %s\n", suffix, right, result);
    break;

  case MUL_OP:
    if (leftReg == resultReg) {
      fprintf(context->file, "    imul%s %s, %s\n", suffix, right, result);
    } else if (rightReg == resultReg) {
      fprintf(context->file, "    imul%s %s, %s\n", suffix, left, result);
    } else {
      fprintf(context->file, "    mov%s %s, %s\n", suffix, left, result);
      fprintf(context->file, "    imul%s %s, %s\n", suffix, right, result);
    }
    break;

  case DIV_OP:
  case MOD_OP:
    if (operandType == TYPE_INT) {
      const char *leftReg32 = getRegisterNameForSize(leftReg, TYPE_INT);
      const char *rightReg32 = getRegisterNameForSize(rightReg, TYPE_INT);

      if (leftReg != REG_RAX) {
        fprintf(context->file, "    movl %s, %%eax\n", leftReg32);
      }
      fprintf(context->file, "    cltd              # Sign extend EAX to EDX:EAX\n");

      if (rightReg == REG_RAX || rightReg == REG_RDX) {
        fprintf(context->file, "    movl %s, %%ecx\n", rightReg32);
        fprintf(context->file, "    idivl %%ecx\n");
      } else {
        fprintf(context->file, "    idivl %s\n", rightReg32);
      }

      if (opType == DIV_OP && resultReg != REG_RAX) {
        const char *resultReg32 = getRegisterNameForSize(resultReg, TYPE_INT);
        fprintf(context->file, "    movl %%eax, %s\n", resultReg32);
      } else if (opType == MOD_OP && resultReg != REG_RDX) {
        const char *resultReg32 = getRegisterNameForSize(resultReg, TYPE_INT);
        fprintf(context->file, "    movl %%edx, %s\n", resultReg32);
      }
    }
    break;

  case EQUAL_OP:
  case NOT_EQUAL_OP:
  case LESS_THAN_OP:
  case GREATER_THAN_OP:
  case LESS_EQUAL_OP:
  case GREATER_EQUAL_OP: {
    if (leftReg != resultReg) {
      fprintf(context->file, "    mov%s %s, %s\n", suffix, left, result);
    }
    fprintf(context->file, "    cmp%s %s, %s\n", suffix, result, right);

    const char *setInstruction;
    switch (opType) {
    case EQUAL_OP:     setInstruction = ASM_SETE;  break;
    case NOT_EQUAL_OP: setInstruction = ASM_SETNE; break;
    case LESS_THAN_OP: setInstruction = ASM_SETL;  break;
    case GREATER_THAN_OP: setInstruction = ASM_SETG; break;
    case LESS_EQUAL_OP: setInstruction = ASM_SETLE; break;
    case GREATER_EQUAL_OP: setInstruction = ASM_SETGE; break;
    default: setInstruction = ASM_SETE; break;
    }

    fprintf(context->file, "    %s %%al\n", setInstruction);
    fprintf(context->file, "    movzbl %%al, %s\n",
            getRegisterNameForSize(resultReg, TYPE_INT));
    break;
  }

  case LOGIC_AND:
    if (leftReg != resultReg && rightReg != resultReg) {
      fprintf(context->file, "    mov%s %s, %s\n", suffix, left, result);
      fprintf(context->file, "    and%s %s, %s\n", suffix, right, result);
    } else if (rightReg == resultReg) {
      fprintf(context->file, "    and%s %s, %s\n", suffix, left, result);
    } else {
      fprintf(context->file, "    and%s %s, %s\n", suffix, right, result);
    }
    break;

  case LOGIC_OR:
    if (leftReg != resultReg && rightReg != resultReg) {
      fprintf(context->file, "    mov%s %s, %s\n", suffix, left, result);
      fprintf(context->file, "    or%s %s, %s\n", suffix, right, result);
    } else if (rightReg == resultReg) {
      fprintf(context->file, "    or%s %s, %s\n", suffix, left, result);
    } else {
      fprintf(context->file, "    or%s %s, %s\n", suffix, right, result);
    }
    break;

  default:
    emitComment(context, "Unknown binary operation");
    break;
  }

  if (invert == 1) {
    fprintf(context->file, "    neg%s %s    # Invert result\n", suffix, result);
  }
}

/**
 * @brief Generates unary operations including arithmetic negation, logical NOT,
 * and increment/decrement. Delegates to specialized float handlers when needed
 * and uses efficient x86-64 instructions like inc/dec for integer operations
 * and test/set patterns for logical operations.
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
 * @brief Generates floating-point binary operations using SSE2 instructions for
 * scalar operations. Handles arithmetic operations and comparisons with proper
 * result placement in XMM or general-purpose registers depending on operation
 * type.
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
 * @brief Generates floating-point unary operations including negation using bit
 * manipulation. Implements IEEE 754 compliant operations by XORing with sign
 * bit mask for negation and handling positive operations as no-ops.
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
