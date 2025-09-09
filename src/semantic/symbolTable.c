#include "symbolTable.h"

#include <stdlib.h>
#include <string.h>

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
    Symbol symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) return NULL;
    symbol->name = malloc(strlen(name) + 1);
    if (symbol->name == NULL) {
        free(symbol);
        return NULL;
    }
    strcpy(symbol->name, name);
    symbol->type = type;
    symbol->line = line;
    symbol->column = column;
    symbol->isInitialized = 0;
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
    free(symbol);
}

/**
 * @brief Creates a new symbol table with optional parent scope.
 *
 * @param parent Parent symbol table (NULL for global scope)
 * @return Newly created SymbolTable or NULL on allocation failure
 */
SymbolTable createSymbolTable(SymbolTable parent) {
    SymbolTable table = malloc(sizeof(SymbolTable));
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


