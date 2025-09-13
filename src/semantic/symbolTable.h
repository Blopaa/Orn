/**
 * @file symbolTable.h
 * @brief Symbol table interface for variable tracking and scope management.
 *
 * Provides a hierarchical symbol table system for managing variable declarations,
 * type information, and scope-based symbol resolution. Supports nested scopes
 * with parent-child relationships for proper variable shadowing and lookup.
 *
 * Key features:
 * - Hierarchical scope management with parent pointers
 * - Symbol storage with type and position information
 * - Scope-aware symbol lookup (current scope first, then parent scopes)
 * - Duplicate symbol detection within same scope
 * - Memory-safe symbol and table management
 */

#ifndef CINTERPRETER_SYMBOLTABLE_H
#define CINTERPRETER_SYMBOLTABLE_H

#include "../parser/parser.h"


/**
 * @brief Data type enumeration for type checking and validation.
 *
 * Represents all supported data types in the language with additional
 * utility types for error handling and unknown type detection.
 */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_UNKNOWN
} DataType;

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
} SymbolType;

typedef struct FunctionParameter {
    char * name;
    DataType type;
    struct FunctionParameter * next;
} * FunctionParameter;

/**
 * @brief Symbol structure representing a declared variable.
 *
 * Contains comprehensive information about each symbol including
 * name, type, position, scope level, and initialization status.
 * Forms a linked list for efficient symbol storage within each scope.
 */
typedef struct Symbol {
    char *name;
    SymbolType symbolType;
    DataType type;
    int line;
    int column;
    int scope;
    int isInitialized; // only for vars
    //only for functions
    FunctionParameter parameters;
    int paramCount;

    struct Symbol *next;
} *Symbol;

/**
 * @brief Symbol table structure for managing symbols within a scope.
 *
 * Represents a single scope level with its own symbol list and optional
 * parent pointer for hierarchical scope management. Enables proper
 * variable shadowing and scope-based symbol resolution.
 */
typedef struct SymbolTable {
    Symbol symbols;
    struct SymbolTable *parent;
    int scope;
    int symbolCount;
} *SymbolTable;

Symbol createSymbol(const char *name, DataType type, int line, int column);

void freeSymbol(Symbol symbol);

SymbolTable createSymbolTable(SymbolTable parent);

void freeSymbolTable(SymbolTable symbolTable);

Symbol addSymbol(SymbolTable symbolTable, const char *name, DataType type, int line, int column);

Symbol lookupSymbol(SymbolTable symbolTable, const char *name);

Symbol lookUpSymbolCurrentOnly(SymbolTable table, const char *name);

DataType getDataTypeFromNode(NodeTypes nodeType);

FunctionParameter createParameter(const char *name, DataType type);

void freeParamList(FunctionParameter paramList);

Symbol addFunctionSymbol(SymbolTable symbolTable, const char *name, DataType returnType,
                        FunctionParameter parameters, int paramCount, int line, int column);

#endif //CINTERPRETER_SYMBOLTABLE_H
