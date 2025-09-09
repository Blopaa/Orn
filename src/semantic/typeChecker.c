//
// Created by pablo on 08/09/2025.
//

#include "typeChecker.h"

#include <stdlib.h>

#include "errorHandling.h"

/**
 * @brief Creates and initializes a new type checking context.
 *
 * Allocates memory for a type checking context and creates the global
 * symbol table. Initializes both global and current scope pointers
 * to the global table for the start of type checking.
 *
 * @return Newly allocated TypeCheckContext or NULL on allocation failure
 *
 * @note The caller is responsible for freeing the context with
 *       freeTypeCheckContext(). Reports specific error codes for
 *       allocation failures.
 */
TypeCheckContext createTypeCheckContext() {
    TypeCheckContext context = malloc(sizeof(struct TypeCheckContext));
    if (context == NULL) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate type check context");
        return NULL;
    };

    context->global = createSymbolTable(NULL);
    if (context->global == NULL) {
        free(context);
        repError(ERROR_SYMBOL_TABLE_CREATION_FAILED, "Failed to create global symbol table");
        return NULL;
    }
    context->current = context->global;

    return context;
}

/**
 * @brief Frees a type checking context and all associated symbol tables.
 *
 * Performs complete cleanup of the type checking context including
 * the global symbol table and all nested symbol tables that may
 * have been created during type checking.
 *
 * @param context Type checking context to free (can be NULL)
 *
 * @note This function only frees the global symbol table. Nested
 *       symbol tables should be freed as scopes are exited during
 *       the type checking process.
 */
void freeTypeCheckContext(TypeCheckContext context) {
    if (context == NULL) return;
    if (context->global != NULL) freeSymbolTable(context->global);
    free(context);
}

/**
 * @brief Checks if two data types are compatible for assignment operations.
 *
 * Implements the type compatibility rules for the language. Supports
 * identity compatibility (same types) and limited implicit conversions.
 * The only implicit conversion allowed is int to float.
 *
 * @param target Target type for assignment
 * @param source Source type being assigned
 * @return 1 if compatible (assignment allowed), 0 if incompatible
 *
 * Compatibility rules:
 * - Same types are always compatible
 * - int can be assigned to float (implicit conversion)
 * - All other type combinations are incompatible
 */
int areCompatible(DataType target, DataType source) {
    if (target == source) return 1;

    switch (target) {
        case TYPE_STRING:
        case TYPE_BOOL:
        case TYPE_INT: return 0;
        case TYPE_FLOAT: return source == TYPE_INT;
        default: return 0;
    }
}

/**
 * @brief Determines the result type of binary operations.
 *
 * Implements type promotion and result type determination for binary
 * operators. Follows C-style type promotion rules for arithmetic
 * operations and validates operand types for logical and comparison
 * operations.
 *
 * @param left Left operand type
 * @param right Right operand type
 * @param op Binary operation node type
 * @return Result type of the operation or TYPE_UNKNOWN for invalid operations
 *
 * Type promotion rules:
 * - Arithmetic: float + int = float, int + int = int
 * - Comparison: operands must be compatible, result is bool
 * - Logical: operands must be bool, result is bool
 */
DataType getOperationResultType(DataType left, DataType right, NodeTypes op) {
    switch (op) {
        case ADD_OP:
        case SUB_OP:
        case MUL_OP:
        case DIV_OP:
        case MOD_OP:
            if (left == TYPE_FLOAT || right == TYPE_FLOAT) return TYPE_FLOAT;
            if (left == TYPE_INT && right == TYPE_INT) return TYPE_INT;
            return TYPE_UNKNOWN;
        case EQUAL_OP:
        case NOT_EQUAL_OP:
        case LESS_EQUAL_OP:
        case GREATER_EQUAL_OP:
        case LESS_THAN_OP:
        case GREATER_THAN_OP:
            if (areCompatible(left, right) || areCompatible(right, left)) return TYPE_BOOL;
            return TYPE_UNKNOWN;
        case LOGIC_AND:
        case LOGIC_OR:
            if (left == TYPE_BOOL && right == TYPE_BOOL) return TYPE_BOOL;
            return TYPE_UNKNOWN;
        default: return TYPE_UNKNOWN;
    }
}

/**
 * @brief Infers the data type of an expression AST node.
 *
 * Recursively analyzes expression nodes to determine their result types.
 * Handles literals, variables, unary operations, binary operations, and
 * validates type correctness for all operations. Performs symbol table
 * lookups for variable references.
 *
 * @param node Expression AST node to analyze
 * @param context Type checking context for symbol resolution
 * @return Inferred DataType or TYPE_UNKNOWN for invalid expressions
 *
 * @note Reports specific error codes for type violations and undefined
 *       variables. The function performs comprehensive type checking
 *       during type inference.
 */
DataType getExpressionType(ASTNode node, TypeCheckContext context) {
    if (node == NULL) return TYPE_UNKNOWN;
    switch (node->nodeType) {
        case INT_LIT: return TYPE_INT;
        case FLOAT_LIT: return TYPE_FLOAT;
        case BOOL_LIT: return TYPE_BOOL;
        case STRING_LIT: return TYPE_STRING;
        case VARIABLE: {
            Symbol symbol = lookupSymbol(context->current, node->value);
            if (symbol == NULL) {
                repError(ERROR_INVALID_EXPRESSION, node->value);
                return TYPE_UNKNOWN;
            }
            if (!symbol->isInitialized) {
                repError(ERROR_VARIABLE_NOT_INITIALIZED, node->value);
                return TYPE_UNKNOWN;
            }
            return symbol->type;
        }
        case UNARY_MINUS_OP:
        case UNARY_PLUS_OP: {
            DataType opType = getExpressionType(node->children, context);
            if (opType == TYPE_INT || opType == TYPE_FLOAT) {
                return opType;
            }
            repError(ERROR_INVALID_UNARY_OPERAND, "Arithmetic unary operators require numeric operands");
            return TYPE_UNKNOWN;
        }
        case LOGIC_NOT: {
            DataType opType = getExpressionType(node->children, context);
            if (opType == TYPE_BOOL) return TYPE_BOOL;
            repError(ERROR_INVALID_UNARY_OPERAND, "Logical NOT requires boolean operand");
            return TYPE_UNKNOWN;
        }
        case PRE_INCREMENT:
        case PRE_DECREMENT:
        case POST_INCREMENT:
        case POST_DECREMENT: {
            DataType operandType = getExpressionType(node->children, context);
            if (operandType == TYPE_INT || operandType == TYPE_FLOAT) {
                return operandType;
            }
            repError(ERROR_INVALID_UNARY_OPERAND, "Increment/decrement operators require numeric operands");
            return TYPE_UNKNOWN;
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
                return TYPE_UNKNOWN;
            }

            DataType leftType = getExpressionType(node->children, context);
            DataType rightType = getExpressionType(node->children->brothers, context);

            return getOperationResultType(leftType, rightType, node->nodeType);
        }

        default:
            return TYPE_UNKNOWN;
    }
}

/**
 * @brief Determines appropriate error code for variable type mismatch scenarios.
 *
 * Maps type mismatch combinations to specific error codes for detailed
 * error reporting. Provides precise error messages for each possible
 * type mismatch scenario in variable assignments.
 *
 * @param varType Expected variable type
 * @param initType Actual initialization/assignment type
 * @return Specific ErrorCode for the type mismatch scenario
 *
 * @note This function assumes the types are incompatible and should
 *       only be called after compatibility checking has failed.
 */
ErrorCode variableErrorCompatibleHandling(DataType varType, DataType initType) {
    ErrorCode err;
    switch (varType) {
        case TYPE_INT: {
            switch (initType) {
                case TYPE_STRING: err = ERROR_TYPE_MISMATCH_STRING_TO_INT;
                    break;
                case TYPE_BOOL: err = ERROR_TYPE_MISMATCH_BOOL_TO_INT;
                    break;
                default: err = ERROR_TYPE_MISMATCH_FLOAT_TO_INT;
            }
            break;
        }
        case TYPE_FLOAT: {
            switch (initType) {
                case TYPE_STRING: err = ERROR_TYPE_MISMATCH_STRING_TO_FLOAT;
                    break;
                default: err = ERROR_TYPE_MISMATCH_BOOL_TO_FLOAT;
            }
            break;
        }
        case TYPE_BOOL: {
            switch (initType) {
                case TYPE_STRING: err = ERROR_TYPE_MISMATCH_STRING_TO_BOOL;
                    break;
                case TYPE_INT: err = ERROR_TYPE_MISMATCH_INT_TO_BOOL;
                    break;
                default: err = ERROR_TYPE_MISMATCH_FLOAT_TO_BOOL;
            }
            break;
        } // TYPE_STRING
        default: {
            switch (initType) {
                case TYPE_INT: err = ERROR_TYPE_MISMATCH_INT_TO_STRING;
                    break;
                case TYPE_FLOAT: err = ERROR_TYPE_MISMATCH_FLOAT_TO_STRING;
                    break;
                default: err = ERROR_TYPE_MISMATCH_BOOL_TO_STRING;
            }
        }
    }
    return err;
}

/**
 * @brief Validates a variable declaration for semantic correctness.
 *
 * Performs comprehensive validation of variable declarations including:
 * - Node structure validation
 * - Type extraction from AST node
 * - Duplicate declaration checking in current scope
 * - Symbol table insertion
 * - Optional initialization type checking
 *
 * @param node Variable declaration AST node
 * @param context Type checking context
 * @return 1 if declaration is valid, 0 if validation failed
 *
 * @note Sets the isInitialized flag if the variable has an initializer.
 *       Only checks for duplicates in the current scope, allowing
 *       variable shadowing in nested scopes.
 */
int validateVariableDeclaration(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->value == NULL) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Variable declaration node is null or has no name");
        return 0;
    };
    DataType varType = getDataTypeFromNode(node->nodeType);
    if (varType == TYPE_UNKNOWN) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Unknown variable type in declaration");
        return 0;
    }

    Symbol existing = lookUpSymbolCurrentOnly(context->current, node->value);
    if (existing != NULL) {
        repError(ERROR_VARIABLE_REDECLARED, node->value);
        return 0;
    }
    Symbol newSymbol = addSymbol(context->current, node->value, varType, node->line, node->column);
    if (newSymbol == NULL) {
        repError(ERROR_SYMBOL_TABLE_CREATION_FAILED, "Failed to add symbol to symbol table");
        return 0;
    }
    if (node->children != NULL) {
        DataType initType = getExpressionType(node->children, context);
        if (initType == TYPE_UNKNOWN) return 0;
        if (!areCompatible(varType, initType)) {
            repError(variableErrorCompatibleHandling(varType, initType), node->value);
            return 0;
        }
        newSymbol->isInitialized = 1;
    }
    return 1;
}

/**
 * @brief Validates assignment operations for type correctness.
 *
 * Performs comprehensive validation of assignment operations including:
 * - Node structure validation (left and right operands)
 * - Left-side assignment target validation (must be variable)
 * - Variable existence checking
 * - Type compatibility between assigned value and variable
 * - Initialization status updates
 *
 * @param node Assignment AST node (ASSIGNMENT or compound assignment)
 * @param context Type checking context
 * @return 1 if assignment is valid, 0 if validation failed
 *
 * @note Handles both simple assignments (=) and compound assignments (+=, -=, etc.).
 *       Updates the isInitialized flag for simple assignments.
 */
int validateAssignment(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->children == NULL || node->children->brothers == NULL) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Assignment node missing operands");
        return 0;
    }
    ASTNode left = node->children;
    ASTNode right = node->children->brothers;

    if (left->nodeType != VARIABLE) {
        repError(ERROR_INVALID_ASSIGNMENT_TARGET, "Left side of assignment must be a variable");
        return 0;
    }
    Symbol symbol = lookupSymbol(context->current, left->value);
    if (symbol == NULL) {
        repError(ERROR_UNDEFINED_VARIABLE, left->value);
        return 0;
    }

    DataType rightType = getExpressionType(right, context);
    if (rightType == TYPE_UNKNOWN) return 0;
    if (!areCompatible(symbol->type, rightType)) {
        repError(variableErrorCompatibleHandling(symbol->type, rightType), left->value);
        return 0;
    }

    if (node->nodeType == ASSIGNMENT) {
        symbol->isInitialized = 1;
    }

    return 1;
}

/**
 * @brief Validates variable usage for declaration and initialization status.
 *
 * Ensures that variables are properly declared and initialized before use.
 * Performs symbol table lookup and validates initialization status to
 * prevent use of uninitialized variables.
 *
 * @param node Variable usage AST node
 * @param context Type checking context
 * @return 1 if variable usage is valid, 0 if validation failed
 *
 * @note This function enforces the language rule that variables must
 *       be initialized before first use. Variables are considered
 *       initialized if they have an initializer in their declaration
 *       or have been assigned a value.
 */
int validateVariableUsage(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->value == NULL) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Variable usage node is null or has no name");
        return 0;
    };

    Symbol symbol = lookupSymbol(context->current, node->value);
    if (symbol == NULL) {
        repError(ERROR_UNDEFINED_VARIABLE, node->value);
        return 0;
    }

    if (!symbol->isInitialized) {
        repError(ERROR_VARIABLE_NOT_INITIALIZED, node->value);
        return 0;
    }

    return 1;
}

/**
 * @brief Recursively type checks all children of an AST node.
 *
 * Traverses all child nodes (linked as siblings) and performs type
 * checking on each one. Accumulates success status to ensure all
 * children pass validation.
 *
 * @param node Parent AST node
 * @param context Type checking context
 * @return 1 if all children passed type checking, 0 if any failed
 *
 * @note This function continues checking all children even if some
 *       fail, allowing for comprehensive error reporting rather than
 *       stopping at the first error.
 */
int typeCheckChildren(ASTNode node, TypeCheckContext context) {
    if (node == NULL) return 1;

    int success = 1;
    ASTNode child = node->children;
    while (child != NULL) {
        if (!typeCheckNode(child, context)) {
            success = 0;
        }
        child = child->brothers;
    }

    return success;
}

/**
 * @brief Recursively type checks a single AST node and its subtree.
 *
 * Performs comprehensive type checking based on the node type. Handles
 * different categories of nodes including declarations, assignments,
 * expressions, statements, and control flow constructs. Manages scope
 * creation and cleanup for block statements.
 *
 * @param node AST node to type check
 * @param context Type checking context
 * @return 1 if type checking passed, 0 if errors occurred
 *
 * @note This is the main dispatch function for type checking. It handles
 *       scope management for blocks and delegates specific validation to
 *       specialized functions for different node types.
 */
int typeCheckNode(ASTNode node, TypeCheckContext context) {
    if (node == NULL) return 1;

    int success = 1;
    switch (node->nodeType) {
        case PROGRAM:
            success = typeCheckChildren(node, context);
            break;

        // Variable declarations
        case INT_VARIABLE_DEFINITION:
        case FLOAT_VARIABLE_DEFINITION:
        case STRING_VARIABLE_DEFINITION:
        case BOOL_VARIABLE_DEFINITION:
            success = validateVariableDeclaration(node, context);
            break;

        case ASSIGNMENT:
        case COMPOUND_ADD_ASSIGN:
        case COMPOUND_SUB_ASSIGN:
        case COMPOUND_MUL_ASSIGN:
        case COMPOUND_DIV_ASSIGN:
            success = validateAssignment(node, context);
            break;

        case BLOCK_STATEMENT:
        case BLOCK_EXPRESSION: {
            SymbolTable oldScope = context->current;
            context->current = createSymbolTable(oldScope);

            if (context->current == NULL) {
                repError(ERROR_SYMBOL_TABLE_CREATION_FAILED, "Failed to create new scope for block");
                success = 0;
                break;
            }

            success = typeCheckChildren(node, context);

            freeSymbolTable(context->current);
            context->current = oldScope;
            break;
        }

        case IF_CONDITIONAL:
        case LOOP_STATEMENT:
        case IF_TRUE_BRANCH:
        case ELSE_BRANCH:
            success = typeCheckChildren(node, context);
            break;

        case VARIABLE:
            success = validateVariableUsage(node, context);
            break;

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
            success = typeCheckChildren(node, context);
            if (success) {
                DataType resultType = getExpressionType(node, context);
                if (resultType == TYPE_UNKNOWN) {
                    success = 0;
                }
            }
            break;
        }

        case UNARY_MINUS_OP:
        case UNARY_PLUS_OP:
        case LOGIC_NOT:
        case PRE_INCREMENT:
        case PRE_DECREMENT:
        case POST_INCREMENT:
        case POST_DECREMENT:
            success = typeCheckChildren(node, context);
            if (success) {
                DataType resultType = getExpressionType(node, context);
                if (resultType == TYPE_UNKNOWN) {
                    success = 0;
                }
            }
            break;

        case INT_LIT:
        case FLOAT_LIT:
        case STRING_LIT:
        case BOOL_LIT:
            break;

        default:
            success = typeCheckChildren(node, context);
            break;
    }
    return success;
}

/**
 * @brief Performs complete type checking on an Abstract Syntax Tree.
 *
 * Main entry point for type checking. Creates a type checking context,
 * performs comprehensive validation of the entire AST, and returns the
 * overall result. Handles context cleanup and integrates with the
 * global error reporting system.
 *
 * @param ast Root AST node (typically PROGRAM node)
 * @return 1 if type checking passed without errors, 0 if errors occurred
 *
 * @note This function integrates with the global error reporting system
 *       and will return 0 if any errors were reported during type checking,
 *       even if the local type checking operations succeeded.
 */
int typeCheckAST(ASTNode ast) {
    if (ast == NULL) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Cannot type check null AST");
        return 0;
    }
    TypeCheckContext context = createTypeCheckContext();
    if (context == NULL) {
        repError(ERROR_INVALID_EXPRESSION, "Failed to create type check context");
        return 0;
    }
    int success = typeCheckNode(ast, context);
    freeTypeCheckContext(context);
    return success && !hasErrors();
}
