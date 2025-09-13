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
BuiltInId resolveOverload(const char *name, DataType argTypes[], int argCount);
int isBuiltinFunction(const char *name);
Symbol findMatchingBuiltinFunction(SymbolTable table, const char *name, DataType argTypes[], int argCount);


#endif // COMPILER_BUILTINS_H
