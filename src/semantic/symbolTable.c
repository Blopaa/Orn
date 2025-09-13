#include "symbolTable.h"
#include "typeChecker.h"

#include <stdlib.h>
#include <string.h>

FunctionParameter createParameter(const char * name, DataType type) {
    if (name == NULL) return NULL;
    FunctionParameter param = malloc(sizeof(struct FunctionParameter));
    if (param == NULL) return NULL;
    param->name = strdup(name);
    if (param->name == NULL) {
        free(param);
        return NULL;
    }
    param->type = type;
    param->next = NULL;
    return param;
}

void freeParamList(FunctionParameter paramList) {
    FunctionParameter current = paramList;
    while (current != NULL) {
        FunctionParameter next = current->next;
        if (current->name) free(current->name);
        free(current);
        current = next;
    }
}

Symbol addFunctionSymbol(SymbolTable symbolTable, const char *name, DataType returnType,
                        FunctionParameter parameters, int paramCount, int line, int column) {
    if (symbolTable == NULL || name == NULL) return NULL;
    Symbol exists = lookupSymbol(symbolTable, name);
    if (exists != NULL) return NULL;

    Symbol newSymbol = malloc(sizeof(struct Symbol));
    if (newSymbol == NULL) return NULL;

    newSymbol->name = strdup(name);
    if (newSymbol->name == NULL) {
        free(newSymbol);
        return NULL;
    }
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

int validateFunctionCall(ASTNode node, TypeCheckContext context) {
    if (node == NULL || node->nodeType != FUNCTION_CALL || node->value == NULL) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Invalid function call node");
        return 0;
    }

    Symbol funcSymbol = lookupSymbol(context->current, node->value);
    if (funcSymbol == NULL) {
        repError(ERROR_UNDEFINED_VARIABLE, node->value);
        return 0;
    }

    if (funcSymbol->symbolType != SYMBOL_FUNCTION) {
        repError(ERROR_INVALID_EXPRESSION, "Attempting to call non-function");
        return 0;
    }

    ASTNode argListNode = node->children;
    if (argListNode == NULL || argListNode->nodeType != ARGUMENT_LIST) {
        repError(ERROR_INTERNAL_PARSER_ERROR, "Function call missing argument list");
        return 0;
    }

    int argCount = 0;
    ASTNode arg = argListNode->children;
    while (arg != NULL) {
        argCount++;
        arg = arg->brothers;
    }

    if (argCount != funcSymbol->paramCount) {
        repError(ERROR_INVALID_EXPRESSION, "Function call argument count mismatch");
        return 0;
    }

    FunctionParameter param = funcSymbol->parameters;
    arg = argListNode->children;

    while (param != NULL && arg != NULL) {
        DataType argType = getExpressionType(arg, context);
        if (argType == TYPE_UNKNOWN) {
            return 0;
        }

        if (!areCompatible(param->type, argType)) {
            repError(variableErrorCompatibleHandling(param->type, argType), param->name);
            return 0;
        }

        param = param->next;
        arg = arg->brothers;
    }

    return 1;
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
            repError(ERROR_INVALID_EXPRESSION, "Non-void function must return a value");
            return 0;
        }
    } else {
        DataType returnType = getExpressionType(node->children, context);
        if (returnType == TYPE_UNKNOWN) {
            return 0;
        }

        if (expectedType == TYPE_VOID) {
            repError(ERROR_INVALID_EXPRESSION, "Void function cannot return a value");
            return 0;
        }

        if (!areCompatible(expectedType, returnType)) {
            repError(variableErrorCompatibleHandling(expectedType, returnType), "return");
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Creates a new symbol with given properties.
 *
 * @param name Symbol name (will be duplicated)
 * @param type Data type of the symbol
 * @param line Line number of declaration
 * @param column Column number of declaration
 * @return Newly created Symbol or NULL on allocation failure
 */
Symbol createSymbol(const char *name, DataType type, int line, int column) {
    if (name == NULL) return NULL;
    Symbol symbol = malloc(sizeof(struct Symbol));
    if (symbol == NULL) return NULL;
    symbol->name = malloc(strlen(name) + 1);
    if (symbol->name == NULL) {
        free(symbol);
        return NULL;
    }
    symbol->symbolType = SYMBOL_VARIABLE;
    strcpy(symbol->name, name);
    symbol->type = type;
    symbol->line = line;
    symbol->column = column;
    symbol->isInitialized = 0;
    symbol->parameters = NULL;
    symbol->paramCount = 0;
    symbol->next = NULL;
    return symbol;
};

/**
 * @brief Frees a symbol and its associated memory.
 *
 * @param symbol Symbol to free (can be NULL)
 */
void freeSymbol(Symbol symbol) {
    if (symbol == NULL) return;
    if (symbol->name) free(symbol->name);
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
 * @param name Symbol name to find
 * @return Found Symbol or NULL if not found in current scope
 */
Symbol lookUpSymbolCurrentOnly(SymbolTable table, const char *name) {
    if (table == NULL || name == NULL) return NULL;
    Symbol current = table->symbols;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
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
 * @return Found Symbol or NULL if not found in any scope
 */
Symbol lookupSymbol(SymbolTable table, const char *name) {
    if (table == NULL || name == NULL) return NULL;

    Symbol found = lookUpSymbolCurrentOnly(table, name);
    if (found != NULL) return found;

    if (table->parent != NULL) {
        return lookupSymbol(table->parent, name);
    }

    return NULL;
}

/**
 * @brief Adds a new symbol to the symbol table.
 *
 * @param table Target symbol table
 * @param name Symbol name
 * @param type Data type
 * @param line Line number of declaration
 * @param column Column number of declaration
 * @return Created Symbol or NULL if symbol already exists or allocation fails
 */
Symbol addSymbol(SymbolTable table, const char *name, DataType type, int line, int column) {
    if (table == NULL || name == NULL) return NULL;

    Symbol existing = lookUpSymbolCurrentOnly(table, name);
    if (existing != NULL) {
        return NULL;
    }

    Symbol newSymbol = createSymbol(name, type, line, column);
    if (newSymbol == NULL) return NULL;

    newSymbol->next = table->symbols;
    table->symbols = newSymbol;

    return newSymbol;
}

/**
 * @brief Converts AST node type to corresponding data type.
 *
 * @param nodeType AST node type from parser
 * @return Corresponding DataType or TYPE_UNKNOWN if not recognized
 */
DataType getDataTypeFromNode(NodeTypes nodeType) {
    switch (nodeType) {
        case INT_LIT:
        case INT_VARIABLE_DEFINITION:
            return TYPE_INT;
        case FLOAT_LIT:
        case FLOAT_VARIABLE_DEFINITION:
            return TYPE_FLOAT;
        case STRING_LIT:
        case STRING_VARIABLE_DEFINITION:
            return TYPE_STRING;
        case BOOL_LIT:
        case BOOL_VARIABLE_DEFINITION:
            return TYPE_BOOL;
        default:
            return TYPE_UNKNOWN;
    }
}


