//
// Created by pablo on 13/09/2025.
//

#ifndef COMPILER_BUILTINS_H
#define COMPILER_BUILTINS_H
#include "symbolTable.h"

typedef enum {
  BUILTIN_PRINT_INT,
  BUILTIN_PRINT_FLOAT,
  BUILTIN_PRINT_DOUBLE, // not yet while size align still being 8 bytes for every type
  BUILTIN_PRINT_STRING,
  BUILTIN_PRINT_BOOL,
  BUILTIN_EXIT,
  BUILTIN_READ_INT,
  BUILTIN_READ_STRING,
  BUILTIN_UNKNOWN
} BuiltInId;

typedef struct BuiltInFunction {
  char* name;
  DataType returnType;
  DataType *paramTypes;
  char **paramNames;
  int paramCount;
  BuiltInId id;
} BuiltInFunction;

void initBuiltIns(SymbolTable globalTable);
BuiltInId resolveOverload(const char *nameStart, size_t nameLength, DataType arg[], int argCount);
int isBuiltinFunction(const char *nameStart, size_t nameLength);
Symbol findMatchingBuiltinFunction(SymbolTable table, const char *nameStart, size_t nameLength,
                                   DataType argTypes[], int argCount);


#endif // COMPILER_BUILTINS_H
