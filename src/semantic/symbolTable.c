#include "symbolTable.h"
#include "typeChecker.h"

#include <stdlib.h>
#include <string.h>
#include "builtIns.h"

FunctionParameter createParameter(const char *nameStart, size_t nameLen, DataType type) {
    FunctionParameter param = malloc(sizeof(struct FunctionParameter));
    if (param == NULL) return NULL;

    param->nameStart = nameStart;
    param->nameLength = nameLen;
    param->type = type;
    param->next = NULL;
    return param;
}

void freeParamList(FunctionParameter paramList) {
    FunctionParameter current = paramList;
    while (current != NULL) {
        FunctionParameter next = current->next;
        free(current);
        current = next;
    }
}

Symbol addFunctionSymbol(SymbolTable symbolTable, const char *nameStart, size_t nameLength,
                         DataType returnType, FunctionParameter parameters, int paramCount,
                         int line, int column) {
    if (symbolTable == NULL || nameStart == NULL) return NULL;

    Symbol exists = lookupSymbol(symbolTable, nameStart, nameLength);
    if (exists != NULL) return NULL;

    Symbol newSymbol = malloc(sizeof(struct Symbol));
    if (newSymbol == NULL) return NULL;

    newSymbol->nameStart = nameStart;
    newSymbol->nameLength = nameLength;
    newSymbol->symbolType = SYMBOL_FUNCTION;
    newSymbol->type = returnType;
    newSymbol->line = line;
    newSymbol->column = column;
    newSymbol->scope = symbolTable->scope;
    newSymbol->isInitialized = 1;
    newSymbol->parameters = parameters;
    newSymbol->paramCount = paramCount;

    newSymbol->next = symbolTable->symbols;
    symbolTable->symbols = newSymbol;
    symbolTable->symbolCount++;

    return newSymbol;
}

//wrapper
Symbol addFunctionSymbolFromNode(SymbolTable symbolTable, ASTNode node, DataType returnType,
                                 FunctionParameter parameters, int paramCount) {
    if (!node) return NULL;
    return addFunctionSymbol(symbolTable, node->start, node->length, returnType,
                           parameters, paramCount, node->line, node->column);
}

//wrapper for builtins
Symbol addFunctionSymbolFromString(SymbolTable symbolTable, const char *name,
                                   DataType returnType, FunctionParameter parameters,
                                   int paramCount, int line, int column) {
    if (!name) return NULL;
    return addFunctionSymbol(symbolTable, name, strlen(name), returnType,
                           parameters, paramCount, line, column);
}

int validateReturnStatement(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->nodeType != RETURN_STATEMENT) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Invalid return statement node");
        return 0;
    }

    if (context->currentFunction == NULL) {
        repError(ERROR_INVALID_EXPRESSION, "Return statement outside function");
        return 0;
    }

    DataType expectedType = context->currentFunction->type;

    if (node->children == NULL) {
        if (expectedType != TYPE_VOID) {
            repError(ERROR_MISSING_RETURN_VALUE, "Non-void function must return a value");
            return 0;
        }
    } else {
        DataType returnType = getExpressionType(node->children, context);
        if (returnType == TYPE_UNKNOWN) {
            return 0;
        }

        if (expectedType == TYPE_VOID) {
            repError(ERROR_UNEXPECTED_RETURN_VALUE, "Void function cannot return a value");
            return 0;
        }

        if (!areCompatible(expectedType, returnType)) {
            repError(ERROR_RETURN_TYPE_MISMATCH, "return");
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Frees a symbol and its associated memory.
 *
 * @param symbol Symbol to free (can be NULL)
 */
void freeSymbol(Symbol symbol) {
    if (symbol == NULL) return;
    if (symbol->symbolType == SYMBOL_FUNCTION && symbol->parameters) {
        freeParamList(symbol->parameters);
    }
    free(symbol);
}

/**
 * @brief Creates a new symbol table with optional parent scope.
 *
 * @param parent Parent symbol table (NULL for global scope)
 * @return Newly created SymbolTable or NULL on allocation failure
 */
SymbolTable createSymbolTable(SymbolTable parent) {
    SymbolTable table = malloc(sizeof(struct SymbolTable));
    if (table == NULL) return NULL;
    table->symbols = NULL;
    table->parent = parent;
    table->scope = (parent == NULL) ? 0 : parent->scope + 1;

    return table;
}

/**
 * @brief Frees a symbol table and all contained symbols.
 *
 * @param table Symbol table to free (can be NULL)
 */
void freeSymbolTable(SymbolTable table) {
    if (table == NULL) return;

    Symbol current = table->symbols;
    while (current != NULL) {
        Symbol next = current->next;
        freeSymbol(current);
        current = next;
    }

    free(table);
}

/**
 * @brief Looks up a symbol only in the current scope (no parent traversal).
 *
 * @param table Symbol table to search
 * @param nameStart Symbol name to find
 * @param nameLength
 * @return Found Symbol or NULL if not found in current scope
 */
Symbol lookupSymbolCurrentOnly(SymbolTable table, const char *nameStart, size_t nameLength) {
    if (table == NULL || nameStart == NULL) return NULL;

    Symbol current = table->symbols;
    while (current != NULL) {
        if (current->nameLength == nameLength &&
            memcmp(current->nameStart, nameStart, nameLength) == 0) {
            return current;
            }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief Looks up a symbol starting from current scope and traversing up parent scopes.
 *
 * @param table Starting scope for lookup
 * @param name Symbol name to find
 * @param nameLength
 * @return Found Symbol or NULL if not found in any scope
 */
Symbol lookupSymbol(SymbolTable table, const char *nameStart, size_t nameLength) {
    if (table == NULL || nameStart == NULL) return NULL;

    Symbol current = table->symbols;
    while (current != NULL) {
        if (current->nameLength == nameLength &&
            memcmp(current->nameStart, nameStart, nameLength) == 0) {
            return current;
            }
        current = current->next;
    }

    if (table->parent != NULL) {
        return lookupSymbol(table->parent, nameStart, nameLength);
    }

    return NULL;
}

/**
 * @brief Adds a new symbol to the symbol table.
 *
 * @param table Target symbol table
 * @param node Symbol name
 * @param type Data type
 * @return Created Symbol or NULL if symbol already exists or allocation fails
 */
Symbol addSymbol(SymbolTable table, const char *nameStart, size_t nameLength,
                 DataType type, int line, int column) {
    if (table == NULL || nameStart == NULL) return NULL;

    Symbol existing = lookupSymbolCurrentOnly(table, nameStart, nameLength);
    if (existing != NULL) return NULL;

    Symbol newSymbol = malloc(sizeof(struct Symbol));
    if (newSymbol == NULL) return NULL;

    newSymbol->nameStart = nameStart;
    newSymbol->nameLength = nameLength;
    newSymbol->symbolType = SYMBOL_VARIABLE;
    newSymbol->type = type;
    newSymbol->line = line;
    newSymbol->column = column;
    newSymbol->scope = table->scope;
    newSymbol->isInitialized = 0;
    newSymbol->parameters = NULL;
    newSymbol->paramCount = 0;
    newSymbol->next = table->symbols;
    table->symbols = newSymbol;

    return newSymbol;
}

// Wrappers
Symbol addSymbolFromNode(SymbolTable table, ASTNode node, DataType type) {
    if (!node) return NULL;
    return addSymbol(table, node->start, node->length, type, node->line, node->column);
}

/**
 * @brief Converts AST node type to corresponding data type.
 *
 * @param nodeType AST node type from parser
 * @return Corresponding DataType or TYPE_UNKNOWN if not recognized
 */
DataType getDataTypeFromNode(NodeTypes nodeType) {
    switch (nodeType) {
        case REF_INT:
        case INT_LIT:
        case INT_VARIABLE_DEFINITION:
            return TYPE_INT;
        case REF_FLOAT:
        case FLOAT_LIT:
        case FLOAT_VARIABLE_DEFINITION:
            return TYPE_FLOAT;
        case REF_STRING:
        case STRING_LIT:
        case STRING_VARIABLE_DEFINITION:
            return TYPE_STRING;
        case REF_BOOL:
        case BOOL_LIT:
        case BOOL_VARIABLE_DEFINITION:
            return TYPE_BOOL;
        default:
            return TYPE_UNKNOWN;
    }
}
