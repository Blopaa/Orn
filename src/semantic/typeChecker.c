//
// Created by pablo on 08/09/2025.
//

#include "typeChecker.h"

#include "builtIns.h"

#include <stdlib.h>
#include <string.h>

#include "errorHandling.h"
#include "semanticHelpers.h"

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
TypeCheckContext createTypeCheckContext(const char *sourceCode, const char *filename) {
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
    context->currentFunction = NULL;
    context->sourceFile = sourceCode;
    context->filename = filename;

    initBuiltIns(context->global);

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

DataType validateMemberAccess(ASTNode node, TypeCheckContext context) {
    if (!node || node->nodeType != MEMBER_ACCESS) return TYPE_UNKNOWN;

    ASTNode objectNode = node->children;
    ASTNode fieldNode = objectNode ? objectNode->brothers : NULL;

    if (!objectNode || !fieldNode) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                   "Invalid member access structure");
        return TYPE_UNKNOWN;
    }

    if (objectNode->nodeType != VARIABLE) {
        reportError(ERROR_INVALID_OPERATION_FOR_TYPE, createErrorContextFromType(node, context),
                   "Member access requires a variable");
        return TYPE_UNKNOWN;
    }

    Symbol objectSymbol = lookupSymbol(context->current, objectNode->start, objectNode->length);
    if (!objectSymbol) {
        reportError(ERROR_UNDEFINED_VARIABLE, createErrorContextFromType(objectNode, context),
                   "Undefined variable in member access");
        return TYPE_UNKNOWN;
    }

    if (objectSymbol->type != TYPE_STRUCT) {
        reportError(ERROR_INVALID_OPERATION_FOR_TYPE, createErrorContextFromType(node, context),
                   "Member access on non-struct type");
        return TYPE_UNKNOWN;
    }

    StructField field = objectSymbol->structType->fields;
    while (field) {
        if (field->nameLength == fieldNode->length &&
            memcmp(field->nameStart, fieldNode->start, fieldNode->length) == 0) {
            return field->type;
            }
        field = field->next;
    }

    reportError(ERROR_UNDEFINED_VARIABLE, createErrorContextFromType(fieldNode, context),
               "Struct has no such field");
    return TYPE_UNKNOWN;
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
            Symbol symbol = lookupSymbol(context->current, node->start, node->length);
            if (symbol == NULL) {
                reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromType(node, context),
                            extractText(node->start, node->length));
                return TYPE_UNKNOWN;
            }
            if (!symbol->isInitialized) {
                reportError(ERROR_VARIABLE_NOT_INITIALIZED, createErrorContextFromType(node, context),
                            extractText(node->start, node->length));
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
            reportError(ERROR_INVALID_UNARY_OPERAND, createErrorContextFromType(node, context),
                        "Arithmetic unary operators require numeric operands");
            return TYPE_UNKNOWN;
        }
        case LOGIC_NOT: {
            DataType opType = getExpressionType(node->children, context);
            if (opType == TYPE_BOOL) return TYPE_BOOL;
            reportError(ERROR_INVALID_UNARY_OPERAND, createErrorContextFromType(node, context),
                        "Logical NOT requires boolean operand");
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
            reportError(ERROR_INVALID_UNARY_OPERAND, createErrorContextFromType(node, context),
                        "Increment/decrement operators require numeric operands");
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
                reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                            "Binary operation missing operands");
                return TYPE_UNKNOWN;
            }

            DataType leftType = getExpressionType(node->children, context);
            DataType rightType = getExpressionType(node->children->brothers, context);
            DataType resultType = getOperationResultType(leftType, rightType, node->nodeType);
            if (resultType == TYPE_UNKNOWN) {
                reportError(ERROR_INCOMPATIBLE_BINARY_OPERANDS, createErrorContextFromType(node, context),
                            "Incompatible types in binary operation");
            }
            return resultType;
        }
        case FUNCTION_CALL: {
            if (isBuiltinFunction(node->start, node->length)) {
                return TYPE_VOID;
            }
            Symbol funcSymbol = lookupSymbol(context->current, node->start, node->length);
            if (funcSymbol != NULL && funcSymbol->symbolType == SYMBOL_FUNCTION) {
                return funcSymbol->type;
            }

            return TYPE_UNKNOWN;
        }
        case MEMBER_ACCESS:
            return validateMemberAccess(node, context);

        default:
            return TYPE_UNKNOWN;
    }
}

int validateFunctionCall(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->nodeType != FUNCTION_CALL || node->start == NULL) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Invalid function call node");
        return 0;
    }

    ASTNode argListNode = node->children;
    if (argListNode == NULL || argListNode->nodeType != ARGUMENT_LIST) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Function call missing argument list");
        return 0;
    }

    if (isBuiltinFunction(node->start, node->length)) {
        return validateBuiltinFunctionCall(node, context);
    }

    return validateUserDefinedFunctionCall(node, context);
}

int validateBuiltinFunctionCall(ASTNode node, TypeCheckContext context) {
    ASTNode argListNode = node->children;

    int argCount = 0;
    ASTNode arg = argListNode->children;
    while (arg != NULL) {
        argCount++;
        arg = arg->brothers;
    }

    DataType *argTypes = NULL;
    if (argCount > 0) {
        argTypes = malloc(argCount * sizeof(DataType));
        if (argTypes == NULL) {
            reportError(ERROR_MEMORY_ALLOCATION_FAILED, createErrorContextFromType(node, context),
                        "Failed to allocate argument types array");
            return 0;
        }

        arg = argListNode->children;
        for (int i = 0; i < argCount && arg != NULL; i++) {
            DataType argType = getExpressionType(arg, context);
            if (argType == TYPE_UNKNOWN) {
                free(argTypes);
                return 0;
            }
            argTypes[i] = argType;
            arg = arg->brothers;
        }
    }

    BuiltInId builtinId = resolveOverload(node->start, node->length, argTypes, argCount);
    int result = (builtinId != BUILTIN_UNKNOWN);

    if (!result) {
        reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromType(node, context),
                    "No matching overload for built-in function");
    }

    if (argTypes != NULL) {
        free(argTypes);
    }

    return result;
}

int validateUserDefinedFunctionCall(ASTNode node, TypeCheckContext context) {
    Symbol funcSymbol = lookupSymbol(context->current, node->start, node->length);
    if (funcSymbol == NULL) {
        reportError(ERROR_UNDEFINED_FUNCTION, createErrorContextFromType(node, context),
                    extractText(node->start, node->length));
        return 0;
    }

    if (funcSymbol->symbolType != SYMBOL_FUNCTION) {
        reportError(ERROR_CALLING_NON_FUNCTION, createErrorContextFromType(node, context),
                    "Attempting to call non-function");
        return 0;
    }

    ASTNode argListNode = node->children;

    // Count arguments
    int argCount = 0;
    ASTNode arg = argListNode->children;
    while (arg != NULL) {
        argCount++;
        arg = arg->brothers;
    }

    // Check argument count
    if (argCount != funcSymbol->paramCount) {
        reportError(ERROR_FUNCTION_ARG_COUNT_MISMATCH, createErrorContextFromType(node, context),
                    "Function call argument count mismatch");
        return 0;
    }

    // Stream validate argument types (no allocation needed!)
    FunctionParameter param = funcSymbol->parameters;
    arg = argListNode->children;

    while (param != NULL && arg != NULL) {
        DataType argType = getExpressionType(arg, context);
        if (argType == TYPE_UNKNOWN) {
            return 0; // Error already reported
        }

        if (!areCompatible(param->type, argType)) {
            reportError(variableErrorCompatibleHandling(param->type, argType),
                        createErrorContextFromType(node, context), extractText(param->nameStart, param->nameLength));
            return 0;
        }

        param = param->next;
        arg = arg->brothers;
    }

    return 1;
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
    if (node == NULL || node->start == NULL) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Variable declaration node is null or has no name");
        return 0;
    };
    DataType varType = getDataTypeFromNode(node->nodeType);
    if (varType == TYPE_UNKNOWN) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Unknown variable type in declaration");
        return 0;
    }

    Symbol existing = lookupSymbolCurrentOnly(context->current, node->start, node->length);
    if (existing != NULL) {
        reportError(ERROR_VARIABLE_REDECLARED, createErrorContextFromType(node, context),
                    extractText(node->start, node->length));
        return 0;
    }
    Symbol newSymbol = addSymbolFromNode(context->current, node, varType);
    if (newSymbol == NULL) {
        reportError(ERROR_SYMBOL_TABLE_CREATION_FAILED, createErrorContextFromType(node, context),
                    "Failed to add symbol to symbol table");
        return 0;
    }
    if (node->children != NULL) {
        DataType initType = getExpressionType(node->children, context);
        if (initType == TYPE_UNKNOWN) return 0;
        if (!areCompatible(varType, initType)) {
            reportError(variableErrorCompatibleHandling(varType, initType), createErrorContextFromType(node, context),
                        extractText(node->start, node->length));
            return 0;
        }
        newSymbol->isInitialized = 1;
    }
    return 1;
}

int validateAssignment(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->children == NULL || node->children->brothers == NULL) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Assignment node missing operands");
        return 0;
    }
    ASTNode left = node->children;
    ASTNode right = node->children->brothers;

    if (left->nodeType != VARIABLE && left->nodeType != MEMBER_ACCESS) {
        reportError(ERROR_INVALID_ASSIGNMENT_TARGET, createErrorContextFromType(node, context),
                    "Left side of assignment must be a variable or member access");
        return 0;
    }

    DataType leftType = getExpressionType(left, context);
    if (leftType == TYPE_UNKNOWN) return 0;

    DataType rightType = getExpressionType(right, context);
    if (rightType == TYPE_UNKNOWN) return 0;

    if (!areCompatible(leftType, rightType)) {
        reportError(variableErrorCompatibleHandling(leftType, rightType), createErrorContextFromType(node, context),
                    "Type mismatch in assignment");
        return 0;
    }

    if (left->nodeType == VARIABLE) {
        Symbol symbol = lookupSymbol(context->current, left->start, left->length);
        if (symbol && node->nodeType == ASSIGNMENT) {
            symbol->isInitialized = 1;
        }
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
    if (node == NULL || node->start == NULL) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Variable usage node is null or has no name");
        return 0;
    };

    Symbol symbol = lookupSymbol(context->current, node->start, node->length);
    if (symbol == NULL) {
        reportError(ERROR_UNDEFINED_VARIABLE, createErrorContextFromType(node, context),
                    extractText(node->start, node->length));
        return 0;
    }

    if (!symbol->isInitialized) {
        reportError(ERROR_VARIABLE_NOT_INITIALIZED, createErrorContextFromType(node, context),
                    extractText(node->start, node->length));
        return 0;
    }

    return 1;
}

FunctionParameter extractParameters(ASTNode paramListNode) {
    if (paramListNode == NULL || paramListNode->nodeType != PARAMETER_LIST) return NULL;

    FunctionParameter firstParam = NULL;
    FunctionParameter lastParam = NULL;

    ASTNode paramNode = paramListNode->children;
    while (paramNode != NULL) {
        if (paramNode->nodeType == PARAMETER && paramNode->length > 0 && paramNode->start != NULL && paramNode->children != NULL) {
            DataType paramType = getDataTypeFromNode(paramNode->children->nodeType);
            FunctionParameter param = createParameter(paramNode->start, paramNode->length, paramType);
            if (param == NULL) {
                freeParamList(firstParam);
                return NULL;
            }
            if (firstParam == NULL) {
                firstParam = param;
            } else {
                lastParam->next = param;
            }
            lastParam = param;
        }
        paramNode = paramNode->brothers;
    }
    return firstParam;
}

DataType getReturnTypeFromNode(ASTNode returnTypeNode) {
    if (returnTypeNode == NULL || returnTypeNode->nodeType != RETURN_TYPE) {
        return TYPE_VOID; // Default to void if no return type specified
    }

    if (returnTypeNode->children != NULL) {
        return getDataTypeFromNode(returnTypeNode->children->nodeType);
    }

    return TYPE_VOID;
}

int validateFunctionDef(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->nodeType != FUNCTION_DEFINITION || node->start == NULL || node->children == NULL) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Invalid function definition node");
        return 0;
    }

    ASTNode paramListNode = node->children;
    ASTNode returnTypeNode = paramListNode ? paramListNode->brothers : NULL;
    ASTNode bodyNode = returnTypeNode ? returnTypeNode->brothers : NULL;

    if (paramListNode == NULL || paramListNode->nodeType != PARAMETER_LIST) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                    "Function missing parameter list");
        return 0;
    }

    FunctionParameter parameters = extractParameters(paramListNode);
    DataType returnType = getReturnTypeFromNode(returnTypeNode);

    int paramCount = 0;
    FunctionParameter param = parameters;
    while (param != NULL) {
        paramCount++;
        param = param->next;
    }

    Symbol funcSymbol = addFunctionSymbolFromNode(context->current, node, returnType,
                                                  parameters, paramCount);
    if (funcSymbol == NULL) {
        reportError(ERROR_VARIABLE_REDECLARED, createErrorContextFromType(node, context),
                    extractText(node->start, node->length));
        freeParamList(parameters);
        return 0;
    }

    SymbolTable oldScope = context->current;
    Symbol oldFunction = context->currentFunction;

    context->current = createSymbolTable(oldScope);
    context->currentFunction = funcSymbol;

    if (context->current == NULL) {
        reportError(ERROR_SYMBOL_TABLE_CREATION_FAILED, createErrorContextFromType(node, context),
                    "Failed to create function scope");
        context->current = oldScope;
        context->currentFunction = oldFunction;
        return 0;
    }

    param = parameters;
    while (param != NULL) {
        Symbol paramSymbol = addSymbol(context->current, param->nameStart, param->nameLength, param->type, node->line,
                                       node->column);
        if (paramSymbol != NULL) {
            paramSymbol->isInitialized = 1;
        }
        param = param->next;
    }

    int success = 1;
    if (bodyNode != NULL) {
        success = typeCheckNode(bodyNode, context);
    }

    freeSymbolTable(context->current);
    context->current = oldScope;
    context->currentFunction = oldFunction;

    return success;
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

StructType createStructType(ASTNode node, TypeCheckContext context) {
    if (!node || node->nodeType != STRUCT_DEFINITION) return NULL;
    StructType structType = malloc(sizeof(struct StructType));
    if (!structType) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "failed to create struct type");
        return NULL;
    };
    structType->nameStart = node->start;
    structType->nameLength = node->length;
    structType->fields = NULL;
    structType->fieldCount = 0;
    structType->size = 0;

    ASTNode fieldList = node->children;
    if (fieldList && fieldList->nodeType == STRUCT_FIELD_LIST) {
        ASTNode field = fieldList->children;
        StructField last = NULL;

        while (field) {
            if (field->nodeType == STRUCT_FIELD && field->children) {
                StructField structField = malloc(sizeof(struct StructField));
                if (!structField) {
                    free(structType);
                    free(structField);
                    return NULL;
                }

                structField->nameStart = field->start;
                structField->nameLength = field->length;
                structField->type = getDataTypeFromNode(field->children->nodeType);
                structField->offset = structType->size;
                structField->next = NULL;

                StructField check = structType->fields;
                while (check) {
                    if (check->nameLength == structField->nameLength && memcmp(check->nameStart, structField->nameStart, check->nameLength) == 0) {
                        reportError(ERROR_VARIABLE_REDECLARED, createErrorContextFromType(field, context), "duplicate field on struct");
                        free(structField);
                        return NULL;
                    }
                    check = check->next;
                }
                size_t fieldSize = 0;
                structType->size += fieldSize;
                structType->fieldCount++;
                if (!structType->fields) {
                    structType->fields = structField;
                }else {
                    last->next = structField;
                }
                last = structField;
            }
            field = field->brothers;
        }
    }
    return structType;
}

int validateStructDef(ASTNode node, TypeCheckContext context) {
    if (!node || node->nodeType != STRUCT_DEFINITION) return 0;

    Symbol exists = lookupSymbolCurrentOnly(context->current, node->start, node->length);
    if (exists) {
        reportError(ERROR_VARIABLE_REDECLARED, createErrorContextFromType(node, context), extractText(node->start, node->length));
        return 0;
    }
    StructType structType = createStructType(node, context);
    if (!structType) return 0;
    Symbol structSymbol = addSymbolFromNode(context->current, node, TYPE_STRUCT);
    if (!structSymbol) {
        free(structType);
        return 0;
    }
    structSymbol->structType = structType;
    structSymbol->symbolType = SYMBOL_TYPE;

    return 1;
}

int validateStructVarDec(ASTNode node, TypeCheckContext context) {
    if (!node || node->nodeType != STRUCT_VARIABLE_DEFINITION) return 0;

    ASTNode typeRef = node->children;
    if (!typeRef || typeRef->nodeType != REF_CUSTOM) {
        reportError(ERROR_INTERNAL_PARSER_ERROR, createErrorContextFromType(node, context),
                   "Invalid struct variable declaration");
        return 0;
    }

    Symbol structSymbol = lookupSymbol(context->current, typeRef->start, typeRef->length);
    if (!structSymbol) {
        reportError(ERROR_UNDEFINED_VARIABLE, createErrorContextFromType(typeRef, context),
                   "Undefined struct type");
        return 0;
    }

    Symbol exists = lookupSymbolCurrentOnly(context->current, node->start, node->length);
    if (exists) {
        reportError(ERROR_VARIABLE_REDECLARED, createErrorContextFromType(node, context),
                  extractText(node->start, node->length));
        return 0;
    }

    Symbol symbol = addSymbolFromNode(context->current, node, TYPE_STRUCT);
    if (!symbol) {
        repError(ERROR_SYMBOL_TABLE_CREATION_FAILED, "Failed to add struct vairable to symbol table");
        return 0;
    }

    symbol->structType = structSymbol->structType;
    if (typeRef->brothers) {
        symbol->isInitialized = 1;
    }

    return 1;
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
        case FUNCTION_DEFINITION:
            success = validateFunctionDef(node, context);
            break;

        case FUNCTION_CALL:
            success = validateFunctionCall(node, context);
            break;

        case RETURN_STATEMENT:
            success = validateReturnStatement(node, context);
            break;
        case PARAMETER_LIST:
        case PARAMETER:
        case ARGUMENT_LIST:
        case RETURN_TYPE:
            success = typeCheckChildren(node, context);
            break;
        case BLOCK_STATEMENT:
        case BLOCK_EXPRESSION: {
            SymbolTable oldScope = context->current;
            context->current = createSymbolTable(oldScope);

            if (context->current == NULL) {
                reportError(ERROR_SYMBOL_TABLE_CREATION_FAILED, createErrorContextFromType(node, context),
                            "Failed to create new scope for block");
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
        case STRUCT_DEFINITION:
            success = validateStructDef(node, context);
            break;
        case STRUCT_VARIABLE_DEFINITION:
            success = validateStructVarDec(node, context);
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
int typeCheckAST(ASTNode ast, const char *sourceCode, const char *filename) {
    TypeCheckContext context = createTypeCheckContext(sourceCode, filename);
    if (context == NULL) {
        repError(ERROR_INVALID_EXPRESSION, "Failed to create type check context");
        return 0;
    }
    int success = typeCheckNode(ast, context);
    freeTypeCheckContext(context);
    return success && !hasErrors();
}
