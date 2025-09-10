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
    if (node == NULL || context == NULL) return 0;

    switch (node->nodeType) {
        case PROGRAM: {
            emitComment(context, "Program start");

            // Generate code for all children (statements)
            ASTNode child = node->children;
            while (child != NULL) {
                if (!generateNodeCode(child, context)) return 0;
                child = child->brothers;
            }
            return 1;
        }

        case INT_VARIABLE_DEFINITION:
        case FLOAT_VARIABLE_DEFINITION:
        case STRING_VARIABLE_DEFINITION:
        case BOOL_VARIABLE_DEFINITION: {
            DataType varType = getDataTypeFromNode(node->nodeType);
            int offset = allocateVariable(context, node->value, varType);

            if (offset == 0) return 0;

            // Initialize with value if provided
            if (node->children != NULL) {
                emitComment(context, "Initialize variable");
                RegisterId initReg;
                if (varType == TYPE_FLOAT) {
                    initReg = generateExpressionToRegister(node->children, context, REG_XMM0);
                } else {
                    initReg = generateExpressionToRegister(node->children, context, REG_RAX);
                }
                generateStoreVariable(context, node->value, initReg);
            }
            return 1;
        }

        case ASSIGNMENT:
        case COMPOUND_ADD_ASSIGN:
        case COMPOUND_SUB_ASSIGN:
        case COMPOUND_MUL_ASSIGN:
        case COMPOUND_DIV_ASSIGN: {
            if (node->children == NULL || node->children->brothers == NULL) return 0;

            ASTNode leftNode = node->children;
            ASTNode rightNode = node->children->brothers;

            if (leftNode->nodeType != VARIABLE) {
                repError(ERROR_INVALID_ASSIGNMENT_TARGET, "Left side must be variable");
                return 0;
            }

            StackVariable var = findStackVariable(context, leftNode->value);
            if (var == NULL) {
                repError(ERROR_UNDEFINED_VARIABLE, leftNode->value);
                return 0;
            }

            RegisterId rightReg;
            if (var->dataType == TYPE_FLOAT) {
                rightReg = generateExpressionToRegister(rightNode, context, REG_XMM0);
            } else {
                rightReg = generateExpressionToRegister(rightNode, context, REG_RAX);
            }

            if (node->nodeType == ASSIGNMENT) {
                generateStoreVariable(context, leftNode->value, rightReg);
            } else {
                // For compound assignments, load current value, perform operation, store back
                RegisterId leftReg;
                if (var->dataType == TYPE_FLOAT) {
                    leftReg = REG_XMM1;
                } else {
                    leftReg = REG_RBX;
                }

                generateLoadVariable(context, leftNode->value, leftReg);

                NodeTypes opType;
                switch (node->nodeType) {
                    case COMPOUND_ADD_ASSIGN: opType = ADD_OP;
                        break;
                    case COMPOUND_SUB_ASSIGN: opType = SUB_OP;
                        break;
                    case COMPOUND_MUL_ASSIGN: opType = MUL_OP;
                        break;
                    case COMPOUND_DIV_ASSIGN: opType = DIV_OP;
                        break;
                    default: opType = ADD_OP;
                        break;
                }

                generateBinaryOp(context, opType, leftReg, rightReg, leftReg, var->dataType);
                generateStoreVariable(context, leftNode->value, leftReg);
            }
            return 1;
        }

        case BLOCK_STATEMENT:
        case BLOCK_EXPRESSION: {
            emitComment(context, "Block start");

            // Generate code for all statements in the block
            ASTNode child = node->children;
            while (child != NULL) {
                if (!generateNodeCode(child, context)) return 0;
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
StackContext createCodeGenContext(const char *file) {
    if (file == NULL) {
        repError(ERROR_INTERNAL_CODE_GENERATOR_ERROR, "No output filename provided");
        return NULL;
    }

    StackContext context = malloc(sizeof(struct StackContext));
    if (context == NULL) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate codegen context");
        return NULL;
    }
    context->file = fopen(file, "w");
    if (context->file == NULL) {
        free(context);
        repError(ERROR_INTERNAL_CODE_GENERATOR_ERROR, "Failed to open output file");
        return NULL;
    }
    context->variable = NULL;
    context->string = NULL; // FIX: Initialize string table
    context->currentOffset = 0;
    context->labelCount = 1;
    context->tempCount = 1;
    context->stringCount = 0; // FIX: Initialize string counter
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
    if (context == NULL) return;

    StackVariable current = context->variable;
    while (current != NULL) {
        StackVariable next = current->next;
        if (current->name) free(current->name);
        free(current);
        current = next;
    }

    StringEntry currentStr = context->string;
    while (currentStr != NULL) {
        StringEntry next = currentStr->next;
        if (currentStr->value) free(currentStr->value);
        if (currentStr->label) free(currentStr->label);
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
void generateLoadVariable(StackContext context, const char *varName, RegisterId reg) {
    StackVariable var = findStackVariable(context, varName);
    if (var == NULL) {
        repError(ERROR_UNDEFINED_VARIABLE, varName);
        return;
    }

    if (var->dataType == TYPE_FLOAT) {
        const char *regName = getFloatRegisterName(reg);
        fprintf(context->file, "    movsd %d(%%rbp), %s    # Load float %s\n",
                var->stackOffset, regName, varName);
    } else {
        const char *regName = getRegisterName(reg, var->dataType);
        fprintf(context->file, "    movq %d(%%rbp), %s    # Load %s\n",
                var->stackOffset, regName, varName);
    }
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
void generateStoreVariable(StackContext context, const char *varName, RegisterId reg) {
    StackVariable var = findStackVariable(context, varName);
    if (var == NULL) {
        repError(ERROR_UNDEFINED_VARIABLE, varName);
        return;
    }

    if (var->dataType == TYPE_FLOAT) {
        const char *regName = getFloatRegisterName(reg);
        fprintf(context->file, "    movsd %s, %d(%%rbp)    # Store float %s\n",
                regName, var->stackOffset, varName);
    } else {
        const char *regName = getRegisterName(reg, var->dataType);
        fprintf(context->file, "    movq %s, %d(%%rbp)    # Store %s\n",
                regName, var->stackOffset, varName);
    }
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
void generateFloatLoadImmediate(StackContext context, const char *value, RegisterId reg) {
    const char *regName = getFloatRegisterName(reg);

    char tempLabel[64];
    snprintf(tempLabel, sizeof(tempLabel), ".FLOAT_%d", context->tempCount++);

    // FIX: Emit float constant properly
    fprintf(context->file, "\n.section .rodata\n");
    fprintf(context->file, "%s:\n", tempLabel);
    fprintf(context->file, "    .double %s\n", value);
    fprintf(context->file, ".text\n");

    fprintf(context->file, "    movsd %s(%%rip), %s    # Load float immediate: %s\n",
            tempLabel, regName, value);
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
void generateStringLoadImmediate(StackContext context, const char *value, RegisterId reg) {
    StringEntry entry = addStringLiteral(context, value);
    if (entry == NULL) return;

    const char *regName = getRegisterName(reg, TYPE_STRING);
    fprintf(context->file, "    leaq %s(%%rip), %s    # Load string: %s\n",
            entry->label, regName, value);
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
void generateLoadImmediate(StackContext context, const char *value, DataType type, RegisterId reg) {
    switch (type) {
        case TYPE_FLOAT:
            generateFloatLoadImmediate(context, value, reg);
            break;

        case TYPE_STRING:
            generateStringLoadImmediate(context, value, reg);
            break;

        case TYPE_BOOL: {
            int boolVal = (strcmp(value, "true") == 0) ? 1 : 0;
            const char *regName = getRegisterName(reg, type);
            fprintf(context->file, "    movq $%d, %s    # Bool: %s\n", boolVal, regName, value);
            break;
        }

        case TYPE_INT: {
            const char *regName = getRegisterName(reg, type);
            fprintf(context->file, "    movq $%s, %s    # Int immediate: %s\n", value, regName, value);
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
void generateFloatBinaryOp(StackContext context, NodeTypes opType, RegisterId leftReg,
                           RegisterId rightReg, RegisterId resultReg) {
    const char *left = getFloatRegisterName(leftReg);
    const char *right = getFloatRegisterName(rightReg);
    const char *result = getFloatRegisterName(resultReg);

    // Move left operand to result register if different
    if (leftReg != resultReg) {
        fprintf(context->file, "    movsd %s, %s\n", left, result);
    }

    switch (opType) {
        case ADD_OP:
            fprintf(context->file, "    addsd %s, %s\n", right, result);
            break;
        case SUB_OP:
            fprintf(context->file, "    subsd %s, %s\n", right, result);
            break;
        case MUL_OP:
            fprintf(context->file, "    mulsd %s, %s\n", right, result);
            break;
        case DIV_OP:
            fprintf(context->file, "    divsd %s, %s\n", right, result);
            break;
        case EQUAL_OP:
        case NOT_EQUAL_OP:
        case LESS_THAN_OP:
        case GREATER_THAN_OP:
        case LESS_EQUAL_OP:
        case GREATER_EQUAL_OP: {
            // Float comparison - result goes to integer register
            const char *intResult = getRegisterName(resultReg, TYPE_INT);
            fprintf(context->file, "    ucomisd %s, %s\n", right, result);

            const char *setInstruction;
            switch (opType) {
                case EQUAL_OP: setInstruction = "sete";
                    break;
                case NOT_EQUAL_OP: setInstruction = "setne";
                    break;
                case LESS_THAN_OP: setInstruction = "setb";
                    break;
                case GREATER_THAN_OP: setInstruction = "seta";
                    break;
                case LESS_EQUAL_OP: setInstruction = "setbe";
                    break;
                case GREATER_EQUAL_OP: setInstruction = "setae";
                    break;
                default: setInstruction = "sete";
                    break;
            }

            fprintf(context->file, "    %s %%al\n", setInstruction);
            fprintf(context->file, "    movzbq %%al, %s\n", intResult);
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
void generateFloatUnaryOp(StackContext context, NodeTypes opType, RegisterId operandReg, RegisterId resultReg) {
    const char *operand = getFloatRegisterName(operandReg);
    const char *result = getFloatRegisterName(resultReg);

    if (operandReg != resultReg) {
        fprintf(context->file, "    movsd %s, %s\n", operand, result);
    }

    switch (opType) {
        case UNARY_MINUS_OP: {
            char negLabel[64];
            snprintf(negLabel, sizeof(negLabel), ".FLOAT_NEG_%d", context->tempCount++);

            fprintf(context->file, "\n.section .rodata\n");
            fprintf(context->file, "%s:\n", negLabel);
            fprintf(context->file, "    .quad 0x8000000000000000\n");
            fprintf(context->file, ".text\n");

            fprintf(context->file, "    xorpd %s(%%rip), %s\n", negLabel, result);
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
void generateStringOperation(StackContext context, NodeTypes opType, RegisterId leftReg,
                             RegisterId rightReg, RegisterId resultReg) {
    const char *left = getRegisterName(leftReg, TYPE_STRING);
    const char *right = getRegisterName(rightReg, TYPE_STRING);
    const char *result = getRegisterName(resultReg, TYPE_INT);

    switch (opType) {
        case EQUAL_OP:
        case NOT_EQUAL_OP: {
            emitComment(context, "String comparison (simplified - pointer comparison)");
            fprintf(context->file, "    cmpq %s, %s    # Compare string pointers\n", right, left);

            const char *setInstruction = (opType == EQUAL_OP) ? "sete" : "setne";
            fprintf(context->file, "    %s %%al\n", setInstruction);
            fprintf(context->file, "    movzbq %%al, %s\n", result);
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
 *
 * @param context Code generation context
 * @param opType Binary operation type
 * @param leftReg Register containing left operand
 * @param rightReg Register containing right operand
 * @param resultReg Register for operation result
 * @param operandType Data type of the operands
 *
 * @note Delegates to specialized functions for floating-point and string
 *       operations while handling integer/boolean operations directly.
 */
void generateBinaryOp(StackContext context, NodeTypes opType, RegisterId leftReg, RegisterId rightReg,
                      RegisterId resultReg, DataType operandType) {
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

    if (leftReg != resultReg) {
        fprintf(context->file, "    movq %s, %s\n", left, result);
    }

    switch (opType) {
        case ADD_OP:
            fprintf(context->file, "    addq %s, %s\n", right, result);
            break;
        case SUB_OP:
            fprintf(context->file, "    subq %s, %s\n", right, result);
            break;
        case MUL_OP:
            fprintf(context->file, "    imulq %s, %s\n", right, result);
            break;
        case DIV_OP:
            fprintf(context->file, "    movq %s, %%rax\n", result);
            fprintf(context->file, "    cqo\n");
            fprintf(context->file, "    idivq %s\n", right);
            if (resultReg != REG_RAX) {
                fprintf(context->file, "    movq %%rax, %s\n", result);
            }
            break;
        case MOD_OP:
            fprintf(context->file, "    movq %s, %%rax\n", result);
            fprintf(context->file, "    cqo\n");
            fprintf(context->file, "    idivq %s\n", right);
            if (resultReg != REG_RDX) {
                fprintf(context->file, "    movq %%rdx, %s\n", result);
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
                case EQUAL_OP: setInstruction = "sete";
                    break;
                case NOT_EQUAL_OP: setInstruction = "setne";
                    break;
                case LESS_THAN_OP: setInstruction = "setl";
                    break;
                case GREATER_THAN_OP: setInstruction = "setg";
                    break;
                case LESS_EQUAL_OP: setInstruction = "setle";
                    break;
                case GREATER_EQUAL_OP: setInstruction = "setge";
                    break;
                default: setInstruction = "sete";
                    break;
            }
            fprintf(context->file, "    cmpq %s, %s\n", right, result);
            fprintf(context->file, "    %s %%al\n", setInstruction);
            fprintf(context->file, "    movzbq %%al, %s\n", result);
            break;
        }
        case LOGIC_AND:
            fprintf(context->file, "    andq %s, %s\n", right, result);
            break;
        case LOGIC_OR:
            fprintf(context->file, "    orq %s, %s\n", right, result);
            break;
        default:
            emitComment(context, "Unknown binary operation");
            break;
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
 * @param preferredReg Preferred output register (may be changed for type compatibility)
 * @return RegisterId containing the expression result
 *
 * @note Float expressions automatically use XMM registers regardless of
 *       the preferred register. Postfix operations preserve the original
 *       value while updating the variable.
 */
RegisterId generateExpressionToRegister(ASTNode node, StackContext context, RegisterId preferredReg) {
    if (node == NULL) return preferredReg;

    switch (node->nodeType) {
        case INT_LIT:
        case FLOAT_LIT:
        case BOOL_LIT: {
            DataType type = getDataTypeFromNode(node->nodeType);
            if (type == TYPE_FLOAT) {
                preferredReg = REG_XMM0; // Use XMM register for floats
            }
            generateLoadImmediate(context, node->value, type, preferredReg);
            return preferredReg;
        }

        case STRING_LIT: {
            generateLoadImmediate(context, node->value, TYPE_STRING, preferredReg);
            return preferredReg;
        }

        case VARIABLE: {
            StackVariable var = findStackVariable(context, node->value);
            if (var && var->dataType == TYPE_FLOAT) {
                preferredReg = REG_XMM0; // Use XMM register for float variables
            }
            generateLoadVariable(context, node->value, preferredReg);
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
                repError(ERROR_INTERNAL_PARSER_ERROR, "Binary operation missing operands");
                return preferredReg;
            }

            // FIX: Determine operand type properly
            DataType operandType = getOperandType(node->children, context);
            if (operandType == TYPE_UNKNOWN) {
                operandType = getOperandType(node->children->brothers, context);
            }

            RegisterId leftReg, rightReg;
            if (operandType == TYPE_FLOAT) {
                leftReg = generateExpressionToRegister(node->children, context, REG_XMM0);
                rightReg = generateExpressionToRegister(node->children->brothers, context, REG_XMM1);
                preferredReg = REG_XMM0;
            } else {
                leftReg = generateExpressionToRegister(node->children, context, REG_RAX);
                rightReg = generateExpressionToRegister(node->children->brothers, context, REG_RBX);
            }

            generateBinaryOp(context, node->nodeType, leftReg, rightReg, preferredReg, operandType);
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

            RegisterId operandReg = generateExpressionToRegister(node->children, context, preferredReg);
            generateUnaryOp(context, node->nodeType, operandReg, preferredReg, operandType);
            return preferredReg;
        }

        case POST_INCREMENT:
        case POST_DECREMENT: {
            if (node->children == NULL || node->children->nodeType != VARIABLE) {
                repError(ERROR_INTERNAL_PARSER_ERROR, "Postfix operation requires variable");
                return preferredReg;
            }

            DataType operandType = getOperandType(node->children, context);
            if (operandType == TYPE_FLOAT) {
                preferredReg = REG_XMM0;
            }

            // Load current value
            generateLoadVariable(context, node->children->value, preferredReg);

            // Store incremented/decremented value back to variable
            RegisterId tempReg = (operandType == TYPE_FLOAT) ? REG_XMM1 : (preferredReg == REG_RAX) ? REG_RBX : REG_RAX;
            generateLoadVariable(context, node->children->value, tempReg);
            generateUnaryOp(context, node->nodeType, tempReg, tempReg, operandType);
            generateStoreVariable(context, node->children->value, tempReg);

            return preferredReg; // Return original value
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
    if (node->children == NULL) return 0;

    char elseLabel[64], endLabel[64];
    generateLabel(context, "else", elseLabel, sizeof(elseLabel));
    generateLabel(context, "end_if", endLabel, sizeof(endLabel));

    // Generate condition
    emitComment(context, "Evaluate condition");
    RegisterId condReg = generateExpressionToRegister(node->children, context, REG_RAX);

    // Test condition and jump to else if false
    fprintf(context->file, "    testq %s, %s\n",
            getRegisterName(condReg, TYPE_INT), getRegisterName(condReg, TYPE_INT));

    ASTNode trueBranch = node->children->brothers;
    ASTNode falseBranch = trueBranch ? trueBranch->brothers : NULL;

    if (falseBranch) {
        fprintf(context->file, "    jz %s\n", elseLabel);
    } else {
        fprintf(context->file, "    jz %s\n", endLabel);
    }

    // Generate true branch
    if (trueBranch && trueBranch->nodeType == IF_TRUE_BRANCH && trueBranch->children) {
        emitComment(context, "True branch");
        generateNodeCode(trueBranch->children, context);
    }

    // Generate false branch if it exists
    if (falseBranch) {
        fprintf(context->file, "    jmp %s\n", endLabel);
        fprintf(context->file, "%s:\n", elseLabel);

        if (falseBranch->nodeType == ELSE_BRANCH && falseBranch->children) {
            emitComment(context, "False branch");
            generateNodeCode(falseBranch->children, context);
        }
    }

    fprintf(context->file, "%s:\n", endLabel);
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
    if (node->children == NULL || node->children->brothers == NULL) return 0;

    char loopLabel[64], endLabel[64];
    generateLabel(context, "loop", loopLabel, sizeof(loopLabel));
    generateLabel(context, "end_loop", endLabel, sizeof(endLabel));

    ASTNode condition = node->children;
    ASTNode body = node->children->brothers;

    fprintf(context->file, "%s:\n", loopLabel);

    // Generate condition
    emitComment(context, "Loop condition");
    RegisterId condReg = generateExpressionToRegister(condition, context, REG_RAX);

    // Test condition and exit if false
    fprintf(context->file, "    testq %s, %s\n",
            getRegisterName(condReg, TYPE_INT), getRegisterName(condReg, TYPE_INT));
    fprintf(context->file, "    jz %s\n", endLabel);

    // Generate loop body
    emitComment(context, "Loop body");
    generateNodeCode(body, context);

    // Jump back to condition
    fprintf(context->file, "    jmp %s\n", loopLabel);
    fprintf(context->file, "%s:\n", endLabel);

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
void generateUnaryOp(StackContext context, NodeTypes opType, RegisterId operandReg, RegisterId resultReg,
                     DataType operandType) {
    if (operandType == TYPE_FLOAT) {
        generateFloatUnaryOp(context, opType, operandReg, resultReg);
        return;
    }

    const char *operand = getRegisterName(operandReg, operandType);
    const char *result = getRegisterName(resultReg, operandType);

    if (operandReg != resultReg) {
        fprintf(context->file, "    movq %s, %s\n", operand, result);
    }

    switch (opType) {
        case UNARY_MINUS_OP:
            fprintf(context->file, "    negq %s\n", result);
            break;
        case UNARY_PLUS_OP:
            break;
        case LOGIC_NOT:
            fprintf(context->file, "    testq %s, %s\n", result, result);
            fprintf(context->file, "    setz %%al\n");
            fprintf(context->file, "    movzbq %%al, %s\n", result);
            break;
        case PRE_INCREMENT:
        case POST_INCREMENT:
            fprintf(context->file, "    incq %s\n", result);
            break;
        case PRE_DECREMENT:
        case POST_DECREMENT:
            fprintf(context->file, "    decq %s\n", result);
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
int generateCode(ASTNode ast, const char *outputFile) {
    // Validate input parameters
    if (ast == NULL) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Cannot generate code from null AST");
        return 0;
    }

    if (outputFile == NULL) {
        repError(ERROR_INTERNAL_CODE_GENERATOR_ERROR, "No output filename provided");
        return 0;
    }

    // Create code generation context
    StackContext context = createCodeGenContext(outputFile);
    if (context == NULL) {
        return 0; // Error already reported in createCodeGenContext
    }

    // Generate assembly preamble (includes string table and program setup)
    emitPreamble(context);

    // Generate code for the entire AST
    int success = generateNodeCode(ast, context);

    // Generate assembly epilogue (program exit code)
    if (success) {
        emitEpilogue(context);
    } else {
        emitComment(context, "Code generation failed - incomplete output");
    }

    // Clean up and close output file
    freeCodegenContext(context);

    return success;
}
