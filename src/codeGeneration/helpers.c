//
// Created by pablo on 10/09/2025.
//

#include <stdlib.h>
#include <string.h>

#include "codeGeneration.h"
#include "errorHandling.h"
#include "asmTemplate.h"

/**
 * @brief Returns the stack allocation size for a given data type.
 *
 * Determines the number of bytes to allocate on the stack for variables
 * of different types. Currently all types use 8 bytes for simplicity
 * and proper x86-64 alignment, following the convention that stack
 * operations should be 8-byte aligned for optimal performance.
 *
 * @param type Data type to get size for
 * @return Number of bytes to allocate on stack
 *
 * @note All types currently use 8-byte allocation:
 *       - INT: 64-bit integer for consistency
 *       - FLOAT: Double precision (64-bit)
 *       - BOOL: 64-bit for alignment efficiency
 *       - STRING: 64-bit pointer
 */
int getStackSize(DataType type) {
    switch (type) {
        case TYPE_INT: return STACK_SIZE_INT;
        case TYPE_FLOAT: return STACK_SIZE_FLOAT;
        case TYPE_BOOL: return STACK_SIZE_BOOL;
        case TYPE_STRING: return STACK_SIZE_STRING;
        default: return STACK_SIZE_INT;
    }
}

/**
 * @brief Returns the assembly instruction suffix for a given data type.
 *
 * Provides the appropriate instruction suffix for x86-64 assembly
 * instructions based on the data type being operated on. Used for
 * generating type-appropriate move and arithmetic instructions.
 *
 * @param type Data type to get suffix for
 * @return Assembly instruction suffix string
 *
 * @note Returns:
 *       - "q" for 64-bit operations (quad word) - INT, BOOL, STRING
 *       - "sd" for scalar double-precision float operations
 */
const char *getAsmTypeSuffix(DataType type) {
    switch (type) {
        case TYPE_INT:
        case TYPE_BOOL:
        case TYPE_STRING: return "q"; // 64-bit (quad word)
        case TYPE_FLOAT: return "sd"; // Scalar double-precision
        default: return "q";
    }
}

/**
 * @brief Maps register IDs to x86-64 assembly register names.
 *
 * Converts internal register enumeration values to their corresponding
 * x86-64 assembly register names. Handles both general-purpose registers
 * and floating-point registers with automatic delegation for float types.
 *
 * @param regId Internal register identifier
 * @param type Data type (used to select between GP and float registers)
 * @return Assembly register name string (e.g., "%rax", "%rbx")
 *
 * @note For floating-point types, automatically delegates to
 *       getFloatRegisterName(). Provides bounds checking and
 *       defaults to "%rax" for invalid register IDs.
 */
const char *getRegisterName(RegisterId regId, DataType type) {
    if (type == TYPE_FLOAT) {
        return getFloatRegisterName(regId);
    }

    static const char *registers[] = {
        ASM_REG_RAX, ASM_REG_RBX, ASM_REG_RCX, ASM_REG_RDX, ASM_REG_RSI,
        ASM_REG_RDI, ASM_REG_R8, ASM_REG_R9, ASM_REG_R10, ASM_REG_R11
    };

    if (regId >= REG_R11 + 1) return "%rax";
    return registers[regId];
}

/**
 * @brief Maps register IDs to SSE register names for floating-point operations.
 *
 * Converts internal register enumeration values to their corresponding
 * SSE register names used for floating-point operations. Provides proper
 * bounds checking and safe defaults for invalid register specifications.
 *
 * @param regId Internal register identifier (should be XMM register)
 * @return SSE register name string (e.g., "%xmm0", "%xmm1")
 *
 * @note Performs bounds checking to ensure valid XMM register range.
 *       Returns "%xmm0" as default for invalid or out-of-range IDs.
 *       Supports XMM0 through XMM5 for current implementation.
 */
const char *getFloatRegisterName(RegisterId regId) {
    static const char *xmmRegisters[] = {
        ASM_REG_XMM0, ASM_REG_XMM1, ASM_REG_XMM2, ASM_REG_XMM3, ASM_REG_XMM4, ASM_REG_XMM5
    };

    int xmmIndex = regId - REG_XMM0;
    if (xmmIndex < 0 || xmmIndex > 5) return ASM_REG_XMM0;
    return xmmRegisters[xmmIndex];
}

/**
 * @brief Generates unique labels for assembly code control flow.
 *
 * Creates unique assembly labels using a prefix and incrementing counter
 * to ensure no label conflicts in the generated code. Used for control
 * flow constructs like loops, conditionals, and temporary data storage.
 *
 * @param context Code generation context (for label counter)
 * @param prefix Descriptive prefix for the label
 * @param buffer Output buffer to store generated label
 * @param bufferSize Size of the output buffer
 *
 * @note Labels follow the format ".L<prefix>_<number>" which is compatible
 *       with GNU assembler local label conventions. The label counter is
 *       automatically incremented to ensure uniqueness.
 */
void generateLabel(StackContext context, const char *prefix, char *buffer, int bufferSize) {
    snprintf(buffer, bufferSize, "%s%s_%d", ASM_LABEL_PREFIX_LOCAL, prefix, context->labelCount++);
}

/**
 * @brief Allocates stack space for a variable and adds it to the context.
 *
 * Manages stack frame allocation by calculating the required space for
 * a variable, updating the stack offset, and creating a StackVariable
 * entry to track the allocation. Emits assembly code to adjust the
 * stack pointer and adds descriptive comments.
 *
 * @param context Code generation context
 * @param start Variable name for tracking and comments
 * @param len
 * @param type Variable data type for size calculation
 * @return Stack offset of allocated variable or 0 on error
 */
int allocateVariable(StackContext context, const char *start, size_t len, DataType type) {
    if (context == NULL || start == NULL) return 0;

    int size = getStackSize(type);
    context->currentOffset += size;

    StackVariable variable = malloc(sizeof(struct StackVariable));
    if (variable == NULL) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate codegen variable");
        return 0;
    }

    variable->stackOffset = -context->currentOffset;
    variable->dataType = type;
    variable->start = start;
    variable->length = len;
    variable->next = context->variable;
    context->variable = variable;

    char * tempName = extractText(start, len);
    if (tempName) {
        ASM_EMIT_COMMENT(context->file, tempName);
        ASM_EMIT_SUBQ_RSP(context->file, size, tempName);
        free(tempName);
    }

    return variable->stackOffset;
}

/**
 * @brief Searches for a stack variable by name in the current context.
 *
 * Performs linear search through the linked list of allocated stack
 * variables to find a variable with the specified name. Used for
 * variable access operations including loads, stores, and type checking.
 *
 * @param context Code generation context containing variable list
 * @param name Variable name to search for
 * @return StackVariable if found, NULL if not found or invalid parameters
 *
 * @note Performs case-sensitive string comparison using strcmp().
 *       Returns NULL safely for invalid parameters (NULL context or name).
 *       Time complexity is O(n) where n is the number of variables.
 */
StackVariable findStackVariable(StackContext context, const char *start, size_t len) {
    if (context == NULL || start == NULL) return NULL;

    StackVariable current = context->variable;
    while (current != NULL) {
        if (current->length == len && memcmp(current->start, start, len ) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief Determines the data type of an AST node operand.
 *
 * Analyzes an AST node to determine its data type, handling both literal
 * nodes (which have inherent types) and variable nodes (which require
 * symbol table lookup). Essential for type-aware code generation.
 *
 * @param node AST node to analyze (can be NULL)
 * @param context Code generation context for symbol table access
 * @return DataType of the operand or TYPE_UNKNOWN if indeterminate
 */
DataType getOperandType(ASTNode node, StackContext context) {
    if (node == NULL) return TYPE_UNKNOWN;

    DataType type = getDataTypeFromNode(node->nodeType);
    if (type == TYPE_UNKNOWN && node->nodeType == VARIABLE) {
        StackVariable var = findStackVariable(context, node->start, node->length);
        if (var) type = var->dataType;
    }
    return type;
}

/**
 * @brief Recursively collects all string literals from an AST.
 *
 * Traverses the entire AST to find and register all string literals
 * in the string table before code generation. Uses depth-first search
 * and handles deduplication automatically.
 *
 * @param node Root AST node to start collection from (can be NULL)
 * @param context Code generation context containing string table
 *
 * @note Must be called before emitPreamble() to ensure complete string table.
 */
void collectStringLiterals(ASTNode node, StackContext context) {
    if (node == NULL) return;

    if (node->nodeType == STRING_LIT) {
        addStringLiteral(context, extractText(node->start, node->length));
    }

    ASTNode child = node->children;
    while (child != NULL) {
        collectStringLiterals(child, context);
        child = child->brothers;
    }
}

/**
 * @brief Checks if an AST node represents a compile-time literal value.
 *
 * Determines whether a node is a literal constant (INT_LIT, FLOAT_LIT,
 * BOOL_LIT, STRING_LIT) that can be loaded as immediate value. Used for
 * code generation optimizations in operand ordering.
 *
 * @param node AST node to test (can be NULL)
 * @return 1 if node is a literal, 0 otherwise
 */
int isLiteral(ASTNode node) {
    if (node == NULL) return 0;
    return (node->nodeType == INT_LIT ||
            node->nodeType == FLOAT_LIT ||
            node->nodeType == BOOL_LIT ||
            node->nodeType == STRING_LIT);
}

/**
 * @brief Spills a register value to the stack.
 *
 * Saves the current contents of a register onto the stack to free it for
 * other computations. Handles both integer/pointer registers and floating-
 * point registers, since floating-point values require specific instructions.
 *
 * @param context Current stack context containing output file stream
 * @param reg     Register identifier to spill
 * @param type    Data type of the value (TYPE_FLOAT or integer/pointer type)
 */
void spillRegisterToStack(StackContext context, RegisterId reg, DataType type) {
    if (type == TYPE_FLOAT) {
        // Float registers need special handling
        fprintf(context->file, ASM_TEMPLATE_SUBQ_RSP, 8, "float", 8);
        fprintf(context->file, ASM_TEMPLATE_MOVSD_REG_MEM,
                getFloatRegisterName(reg), 0, "spill float");
    } else {
        // Integer/pointer registers
        fprintf(context->file, "    %s %s          # Spill to stack\n",
                ASM_PUSHQ, getRegisterName(reg, type));
    }
    ASM_EMIT_COMMENT(context->file, "Saved intermediate result to stack");
}

/**
 * @brief Restores a register value from the stack.
 *
 * Loads a previously spilled register value from the stack back into the
 * specified register. Correctly handles both integer/pointer registers and
 * floating-point registers. Also updates the stack pointer after restoring.
 *
 * @param context Current stack context containing output file stream
 * @param reg     Register identifier to restore into
 * @param type    Data type of the value (TYPE_FLOAT or integer/pointer type)
 */
void restoreRegisterFromStack(StackContext context, RegisterId reg, DataType type) {
    if (type == TYPE_FLOAT) {
        // Float registers need special handling
        fprintf(context->file, ASM_TEMPLATE_MOVSD_MEM_REG,
                0, getFloatRegisterName(reg), "restore float");
        fprintf(context->file, "    %s $8, %s     # Deallocate stack space\n",
                ASM_ADDQ, ASM_REG_RSP);
    } else {
        // Integer/pointer registers
        fprintf(context->file, "    %s %s           # Restore from stack\n",
                ASM_POPQ, getRegisterName(reg, type));
    }
    ASM_EMIT_COMMENT(context->file, "Restored intermediate result from stack");
}

RegisterId getOppositeBranchRegister(RegisterId reg) {
    switch (reg) {
        case REG_RAX: return REG_RBX;
        case REG_RBX: return REG_RAX;
        case REG_XMM0: return REG_XMM1;
        default: return REG_XMM0;
    }
}
