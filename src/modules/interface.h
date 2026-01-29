#ifndef INTERFACE_H
#define INTERFACE_H

#include "../parser/parser.h"
#include "../semantic/symbolTable.h"
#include "../semantic/typeChecker.h"

typedef struct ExportedFunction {
    char *name;
    char *signature;
    char *returnType;
    struct ExportedFunction *next;
} ExportedFunction;

typedef struct ModuleInterface {
    char *moduleName;
    ExportedFunction *functions;
    int functionCount;
} ModuleInterface;

ModuleInterface *extractExportsWithContext(ASTNode ast, const char *moduleName,
                                           TypeCheckContext ctx);

/**
 * @brief Add imported functions to symbol table
 */
int addImportsToSymbolTable(SymbolTable table, ModuleInterface *iface);

/**
 * @brief Free module interface
 */
void freeModuleInterface(ModuleInterface *iface);

/**
 * @brief Convert DataType to string for .orni output
 */
const char *dataTypeToString(DataType type);

/**
 * @brief Parse type string back to DataType
 */
DataType stringToDataType(const char *str);

#endif // INTERFACE_H
