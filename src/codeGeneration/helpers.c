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
StackSize getStackSize(DataType type) {
    switch (type) {
        case TYPE_INT: return STACK_SIZE_INT;
        case TYPE_FLOAT: return STACK_SIZE_FLOAT;
        case TYPE_BOOL: return STACK_SIZE_BOOL;
        case TYPE_STRING: return STACK_SIZE_STRING;
        case TYPE_STRUCT: return STACK_SIZE_STRING;
        case TYPE_DOUBLE: return STACK_SIZE_DOUBLE;
        default: return STACK_SIZE_INT;
    }
}

int alignSize(DataType type) {
    return alignTo(getStackSize(type), 8);
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

    if (regId >= REG_R11 + 1) return ASM_REG_RAX;
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
    int oldOffset = context->currentOffset;
    context->currentOffset = alignTo(context->currentOffset, size);
    context->currentOffset += size;

    int bytesToAlloc = context->currentOffset - oldOffset;

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
        fprintf(context->file, ASM_TEMPLATE_SUBQ_RSP,
                bytesToAlloc, tempName, size);
        free(tempName);
    }

    return variable->stackOffset;
}

const char *getInstructionSuffix(DataType type) {
    switch (type) {
    case TYPE_INT: return "l";      // 32-bit (long)
    case TYPE_FLOAT: return "ss";   // Single precision float
    case TYPE_BOOL: return "b";     // Byte
    case TYPE_STRING: return "q";   // Quad word (64-bit pointer)
    default: return "l";
    }
}

const char *getRegisterNameForSize(RegisterId regId, DataType type) {
    static const char *registers64[] = {"%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"};
    static const char *registers32[] = {"%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "%r8d", "%r9d", "%r10d", "%r11d"};
    static const char *registers8[] = {"%al", "%bl", "%cl", "%dl", "%sil", "%dil", "%r8b", "%r9b", "%r10b", "%r11b"};

    if (regId >= REG_R11 + 1) return registers64[0];

    switch (type) {
    case TYPE_INT:
    case TYPE_FLOAT: return registers32[regId];
    case TYPE_BOOL: return registers8[regId];
    case TYPE_STRING:
    default: return registers64[regId];
    }
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
    }else if (type == TYPE_UNKNOWN && node->nodeType == MEMBER_ACCESS) {
        ASTNode objNode = node->children;
        ASTNode fieldNode = objNode ? objNode->brothers : NULL;
        if (objNode && fieldNode) {
            StackVariable var = findStackVariable(context, objNode->start, objNode->length);
            if (var && var->dataType == TYPE_STRUCT) {
                StructField field = findStructField(var->structType, fieldNode->start, fieldNode->length);
                if (field) {
                    type = field->type;
                }
            }
        }
    }else if (type == TYPE_UNKNOWN && node->nodeType == FUNCTION_CALL) {
        if (context->symbolTable) {
            Symbol funcSymbol = lookupSymbol(context->symbolTable, node->start, node->length);
            if (funcSymbol && funcSymbol->symbolType == SYMBOL_FUNCTION) {
                type = funcSymbol->type;
            }
        }
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
        char * tempVal = extractText(node->start, node->length);
        addStringLiteral(context, tempVal);
        if (tempVal) free(tempVal);
    }

    ASTNode child = node->children;
    while (child != NULL) {
        collectStringLiterals(child, context);
        child = child->brothers;
    }
}

FloatDoubleEntry addFloatDoubleLiterals(StackContext context, const char *value, DataType type) {
    if (context == NULL || value == NULL) return NULL;

    FloatDoubleEntry current = context->floatDoubleEntries;
    while (current != NULL) {
        if (strcmp(current->value, value) == 0 && current->type == type) {
            return current;
        }
        current = current->next;
    }

    FloatDoubleEntry entry = malloc(sizeof(struct FloatDoubleEntry));
    if (entry == NULL) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate float entry");
        return NULL;
    }

    entry->value = strdup(value);
    entry->type = type;
    entry->index = context->floatDoubleCount++;

    entry->label = malloc(32);
    if (type == TYPE_FLOAT) {
        snprintf(entry->label, 32, "%s%d", ASM_LABEL_PREFIX_FLOAT, entry->index);
    } else {
        snprintf(entry->label, 32, "%s%d", ASM_LABEL_PREFIX_DOUBLE, entry->index);
    }

    entry->next = context->floatDoubleEntries;
    context->floatDoubleEntries = entry;

    return entry;
}

FloatDoubleEntry findFloatDoubleLiteral(StackContext context, const char *value, DataType type) {
    if (context == NULL || value == NULL) return NULL;
    
    FloatDoubleEntry current = context->floatDoubleEntries;
    while (current != NULL) {
        if (strcmp(current->value, value) == 0 && current->type == type) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void collectFloatLiterals(ASTNode node, StackContext context) {
    if (node == NULL) return;

    if (node->nodeType == FLOAT_LIT || node->nodeType == DOUBLE_LIT) {
        char *tempVal = extractText(node->start, node->length);
        if(tempVal){
            size_t len = strlen(tempVal);
             if (len > 0 && (tempVal[len-1] == 'f' || tempVal[len-1] == 'F')) {
                tempVal[len-1] = '\0';
            }
        }
        DataType dataType = getDataTypeFromNode(node->nodeType); 
        addFloatDoubleLiterals(context, tempVal, dataType);    
        if (tempVal) free(tempVal);
    }

    ASTNode child = node->children;
    while (child != NULL) {
        collectFloatLiterals(child, context);
        child = child->brothers;
    }
}

int isLiteral(ASTNode node) {
    if (node == NULL) return 0;
    return node->nodeType == INT_LIT ||
            node->nodeType == FLOAT_LIT ||
            node->nodeType == BOOL_LIT ||
            node->nodeType == STRING_LIT;
}

int isLeafNode(ASTNode node) {
    if (node == NULL) return 0;
    return  isLiteral(node) || node->nodeType == VARIABLE;
}

/**
 * @brief Spills a register value to a temporary variable on the stack.
 * Handles both integer and floating-point registers with appropriate instructions.
 */
void spillRegisterToTempVar(StackContext context, RegisterId reg, DataType type, tempVarOffset tempVarOffset) {
    if (type == TYPE_FLOAT) {
        fprintf(context->file, ASM_TEMPLATE_SPILL_FLOAT,
                getFloatRegisterName(reg), tempVarOffset);
    } else {
        const char *regName = getRegisterNameForSize(reg, type);
        const char *suffix = getInstructionSuffix(type);
        fprintf(context->file, ASM_TEMPLATE_SPILL_REG,
               suffix, regName, tempVarOffset);
    }
    ASM_EMIT_COMMENT(context->file, "Saved intermediate result to tempVar");
}

/**
 * @brief Restores a register value from a temporary variable on the stack.
 * Loads previously spilled value back into the specified register.
 */
void restoreRegisterFromTempVar(StackContext context, RegisterId reg, DataType type, tempVarOffset tempVarOffset) {
    if (type == TYPE_FLOAT) {
        // Float registers need special handling
        fprintf(context->file, ASM_TEMPLATE_MOVSD_MEM_REG,
                0, getFloatRegisterName(reg), "restore float");
        fprintf(context->file, "    %s $8, %s     # Deallocate stack space\n",
                ASM_ADDQ, ASM_REG_RSP);
    } else {
        // Integer/pointer registers
        const char *regName = getRegisterNameForSize(reg, type);
        const char *suffix = getInstructionSuffix(type);
        fprintf(context->file, ASM_TEMPLATE_RESTORE_REG,
               suffix, tempVarOffset, regName);
    }
    ASM_EMIT_COMMENT(context->file, "Restored intermediate result from tempVar");
}

/**
 * @brief Returns an alternative register for binary operations when register conflicts occur.
 * Maps between RAX/RBX and XMM0/XMM1 pairs to avoid overwriting operands during
 * complex expression evaluation with intermediate value storage.
 */
RegisterId getOppositeBranchRegister(RegisterId reg) {
    switch (reg) {
        case REG_RAX: return REG_RBX;
        case REG_RBX: return REG_RAX;
        case REG_XMM0: return REG_XMM1;
        default: return REG_XMM0;
    }
}

/**
 * @brief Creates error context for code generation phase with source line extraction.
 * Builds error reporting context from AST node position and extracts the corresponding
 * source line from the original file for detailed error messages.
 */
ErrorContext *createErrorContextFromCodeGen(ASTNode node, StackContext context) {
    static ErrorContext errorContext;
    static char *lastSourceLine = NULL;

    if (!node || !context) return NULL;

    if (lastSourceLine) {
        free(lastSourceLine);
        lastSourceLine = NULL;
    }

    if (context->sourceFile) {
        const char *lineStart = context->sourceFile;
        int currentLine = 1;

        while (*lineStart && currentLine < node->line) {
            if (*lineStart == '\n') {
                currentLine++;
            }
            lineStart++;
        }

        const char *lineEnd = lineStart;
        while (*lineEnd && *lineEnd != '\n') {
            lineEnd++;
        }

        size_t lineLength = lineEnd - lineStart;
        lastSourceLine = malloc(lineLength + 1);
        if (lastSourceLine) {
            strncpy(lastSourceLine, lineStart, lineLength);
            lastSourceLine[lineLength] = '\0';
        }
    }

    errorContext.file = context->filename ? context->filename : "source";
    errorContext.line = node->line;
    errorContext.column = node->column;
    errorContext.source = lastSourceLine;
    errorContext.length = node->length;
    errorContext.startColumn = node->column;

    return &errorContext;
}

/**
 * @brief Searches global symbol table for struct type definitions by name.
 * Looks up struct types in the global scope to enable struct variable declarations
 * and validates that the symbol is actually a struct type definition.
 */
StructType findGlobalStructType(StackContext context, const char * start, size_t len) {
    if (!context || !start || !context->symbolTable) return NULL;
    Symbol structSymbol = lookupSymbol(context->symbolTable, start, len);
    if (!structSymbol || structSymbol->symbolType != SYMBOL_TYPE || structSymbol->type != TYPE_STRUCT) {
        return NULL;
    }
    return structSymbol->structType;
}

int calcStructSize(StructType structType) {
    if (structType == NULL) return 8;
    int currentOffset = 0;
    int maxAlignment = 1;

    StructField field = structType->fields;
    while (field != NULL) {
        int fieldSize = getStackSize(field->type);

        if (fieldSize > maxAlignment) {
            maxAlignment = fieldSize;
        }

        currentOffset = alignTo(currentOffset, fieldSize);
        field->offset = currentOffset;
        currentOffset += fieldSize;

        field = field->next;
    }

    int structSize = alignTo(currentOffset, maxAlignment);
    return alignTo(structSize, 8);
}

/**
 * @brief Locates a specific field within a struct type definition by name.
 * Searches through the struct's field list to find a field with matching name
 * and returns the field information including type and offset data.
 */
StructField findStructField(StructType structType, const char * start, size_t len) {
    if (!structType || !start) return NULL;
    StructField field = structType->fields;
    while (field != NULL) {
        if (field->nameLength == len && memcmp(field->nameStart, start, len) == 0) {
            return field;
        }
        field = field->next;
    }
    return NULL;
}

/**
 * @brief Allocates stack space for a struct variable and tracks it in the context.
 * Creates a stack variable entry for a struct instance, calculates required space,
 * and emits assembly code to reserve the memory on the stack frame.
 */
int allocateStructVariable(StackContext context, const char *start, size_t len,
                           StructType structType) {
    if (!context || !start || !structType) return 0;
    int size = calcStructSize(structType);
    int oldOffset = context->currentOffset;
    context->currentOffset = alignTo(context->currentOffset, 8);
    context->currentOffset += size;
    int bytesToAlloc = context->currentOffset - oldOffset;
    StackVariable variable = malloc(sizeof(struct StackVariable));
    if (!variable) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate struct variable");
        return 0;
    }

    variable->stackOffset = -context->currentOffset;
    variable->dataType = TYPE_STRUCT;
    variable->structType = structType;
    variable->start = start;
    variable->length = len;
    variable->next = context->variable;
    context->variable = variable;

    char *tempName = extractText(start, len);
    if (tempName) {
        fprintf(context->file, "    # Allocate struct %s (size=%d)\n", tempName, size);
        fprintf(context->file, "    subq $%d, %%rsp    # Allocate struct %s (size=%d)\n",
                bytesToAlloc, tempName, size);
        free(tempName);
    }

    return variable->stackOffset;
}

int alignTo(int val, int alignement) {
    return (val + alignement - 1) & ~(alignement - 1);
}