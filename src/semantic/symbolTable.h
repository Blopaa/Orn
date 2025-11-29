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

struct TypeCheckContext;
typedef struct TypeCheckContext *TypeCheckContext;

/**
 * @brief Data type enumeration for type checking and validation.
 *
 * Represents all supported data types in the language with additional
 * utility types for error handling and unknown type detection.
 */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_STRUCT,
    TYPE_UNKNOWN
} DataType;

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
} SymbolType;

typedef struct FunctionParameter {
    const char *nameStart;
    size_t nameLength;
    DataType type;
    struct FunctionParameter *next;
} *FunctionParameter;

typedef struct StructField {
    const char *nameStart;
    size_t nameLength;
    DataType type;
    size_t offset;
    struct StructField *next;
} * StructField;

typedef struct StructType {
    const char * nameStart;
    size_t nameLength;
    StructField fields;
    size_t size;
    int fieldCount;
} *StructType;

// gotta redo this with unions

/**
 * @brief Symbol structure representing a declared variable.
 *
 * Contains comprehensive information about each symbol including
 * name, type, position, scope level, and initialization status.
 * Forms a linked list for efficient symbol storage within each scope.
 */
typedef struct Symbol {
    const char *nameStart;
    uint16_t nameLength;
    SymbolType symbolType;
    DataType type;
    union {
        StructType structType; // only for structs
        struct {
            // only for functions
            FunctionParameter parameters;
            int paramCount;
        };
        struct {
            // only for vars
            int isInitialized;
            int isConst; 
        };
    };
    int line;
    int column;
    int scope;
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

Symbol createSymbol(ASTNode node, DataType type);

void freeSymbol(Symbol symbol);

SymbolTable createSymbolTable(SymbolTable parent);

void freeSymbolTable(SymbolTable symbolTable);

Symbol addSymbolFromString(SymbolTable table, const char *name, DataType type,
                          int line, int column);

Symbol addSymbolFromNode(SymbolTable table, ASTNode node, DataType type) ;

Symbol addSymbol(SymbolTable table, const char *nameStart, size_t nameLength,
                 DataType type, int line, int column);

Symbol lookupSymbol(SymbolTable symbolTable, const char *name, size_t len);

Symbol lookupSymbolCurrentOnly(SymbolTable table, const char *nameStart, size_t nameLength);

DataType getDataTypeFromNode(NodeTypes nodeType);

FunctionParameter createParameter(const char *nameStart, size_t nameLen, DataType type);

void freeParamList(FunctionParameter paramList);

Symbol addFunctionSymbolFromNode(SymbolTable symbolTable, ASTNode node, DataType returnType,
                                 FunctionParameter parameters, int paramCount);

Symbol addFunctionSymbolFromString(SymbolTable symbolTable, const char *name,
                                   DataType returnType, FunctionParameter parameters,
                                   int paramCount, int line, int column);

#endif //CINTERPRETER_SYMBOLTABLE_H
