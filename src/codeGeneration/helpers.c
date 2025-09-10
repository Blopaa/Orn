//
// Created by pablo on 10/09/2025.
//

#include <stdlib.h>
#include <string.h>

#include "codeGeneration.h"
#include "errorHandling.h"

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
        "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"
    };

    if (regId > REG_R11) return "%rax";
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
        "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5"
    };

    int xmmIndex = regId - REG_XMM0;
    if (xmmIndex < 0 || xmmIndex > 5) return "%xmm0";
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
    snprintf(buffer, bufferSize, ".L%s_%d", prefix, context->labelCount++);
}

/**
 * @brief Allocates stack space for a variable and adds it to the context.
 *
 * Manages stack frame allocation by calculating the required space for
 * a variable, updating the stack offset, and creating a StackVariable
 * entry to track the allocation. Emits assembly code to adjust the
 * stack pointer and adds descriptive comments.
 *
 * Process:
 * 1. Calculate space required based on variable type
 * 2. Update current stack offset
 * 3. Create and initialize StackVariable structure
 * 4. Link variable into context's variable list
 * 5. Emit stack allocation assembly code with comments
 *
 * @param context Code generation context
 * @param name Variable name for tracking and comments
 * @param type Variable data type for size calculation
 * @return Stack offset of allocated variable or 0 on error
 *
 * @note Stack offsets are negative (growing downward from frame pointer).
 *       Reports ERROR_MEMORY_ALLOCATION_FAILED for allocation failures.
 *       Emits assembly comments documenting each allocation.
 */
int allocateVariable(StackContext context, const char *name, DataType type) {
    if (context == NULL || name == NULL) return 0;

    int size = getStackSize(type);
    context->currentOffset += size;

    StackVariable variable = malloc(sizeof(struct StackVariable));
    if (variable == NULL) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate codegen variable");
        return 0;
    }

    variable->stackOffset = -context->currentOffset;
    variable->dataType = type;
    variable->name = strdup(name);
    variable->next = context->variable;
    context->variable = variable;

    emitComment(context, name);
    fprintf(context->file, "    subq $%d, %%rsp    # Allocate %s (%d bytes)\n",
            size, name, size);

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
StackVariable findStackVariable(StackContext context, const char *name) {
    if (context == NULL || name == NULL) return NULL;

    StackVariable current = context->variable;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

DataType getOperandType(ASTNode node, StackContext context) {
    if (node == NULL) return TYPE_UNKNOWN;

    DataType type = getDataTypeFromNode(node->nodeType);
    if (type == TYPE_UNKNOWN && node->nodeType == VARIABLE) {
        StackVariable var = findStackVariable(context, node->value);
        if (var) type = var->dataType;
    }
    return type;
}