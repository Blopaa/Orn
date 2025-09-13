//
// Created by pablo on 10/09/2025.
//

#ifndef CINTERPRETER_CODEGENERATION_H
#define CINTERPRETER_CODEGENERATION_H
#include <stdio.h>

#include "symbolTable.h"

/**
 * @brief String literal entry in the global string table.
 *
 * Manages string literals used throughout the program with automatic
 * deduplication and label generation. Forms a linked list of all
 * string literals encountered during code generation.
 */
typedef struct StringEntry {
    char *value;
    char *label;
    int index;
    struct StringEntry *next;
} *StringEntry;

/**
 * @brief Stack variable representation for code generation.
 *
 * Tracks variables allocated on the stack with their memory layout,
 * type information, and linkage. Forms a linked list of all variables
 * in the current compilation scope.
 */
typedef struct StackVariable {
    int stackOffset;
    DataType dataType;
    char *name;
    struct StackVariable *next;
} *StackVariable;

/**
 * @brief Code generation context maintaining compilation state.
 *
 * Central data structure that tracks all state information during
 * code generation including output file, variable allocations,
 * string literals, and generation counters for unique naming.
 */
typedef struct StackContext {
    FILE *file;
    StackVariable variable;
    StringEntry string;
    int currentOffset;
    int labelCount;
    int tempCount;
    int stringCount;
} *StackContext;

/**
 * @brief Stack size constants for different data types.
 *
 * Defines the stack space allocation for each supported data type.
 * Currently all types use 8 bytes for simplicity and alignment,
 * following x86-64 conventions for efficient memory access.
 */
typedef enum {
    STACK_SIZE_INT = 8,
    STACK_SIZE_FLOAT = 8,
    STACK_SIZE_BOOL = 8,
    STACK_SIZE_STRING = 8
} StackSize;

/**
 * @brief Register enumeration for x86-64 and SSE register allocation.
 *
 * Defines all available registers for code generation including
 * general-purpose registers (RAX-R11) and SSE registers (XMM0-XMM5)
 * for floating-point operations. Used for register allocation and
 * instruction generation throughout the compilation process.
 */
typedef enum {
    REG_RAX = 0,
    REG_RBX = 1,
    REG_RCX = 2,
    REG_RDX = 3,
    REG_RSI = 4,
    REG_RDI = 5,
    REG_R8 = 6,
    REG_R9 = 7,
    REG_R10 = 8,
    REG_R11 = 9,
    // XMM registers for floating point
    REG_XMM0 = 10,
    REG_XMM1 = 11,
    REG_XMM2 = 12,
    REG_XMM3 = 13,
    REG_XMM4 = 14,
    REG_XMM5 = 15
} RegisterId;

int generateNodeCode(ASTNode node, StackContext context);

int generateCode(ASTNode ast, const char *outputFile);


StringEntry addStringLiteral(StackContext context, const char *value);

void emitStringTable(StackContext context);


const char *getRegisterName(RegisterId regId, DataType type);

const char *getFloatRegisterName(RegisterId regId);

void generateFloatLoadImmediate(StackContext context, const char *value, RegisterId reg);

void generateFloatBinaryOp(StackContext context, NodeTypes opType, RegisterId leftReg,
                           RegisterId rightReg, RegisterId resultReg);

void generateFloatUnaryOp(StackContext context, NodeTypes opType, RegisterId operandReg,
                          RegisterId resultReg);

void generateStringLoadImmediate(StackContext context, const char *value, RegisterId reg);

void generateStringOperation(StackContext context, NodeTypes opType, RegisterId leftReg,
                             RegisterId rightReg, RegisterId resultReg);

void generateLabel(StackContext context, const char *prefix, char *buffer, int bufferSize);

void emitComment(StackContext context, const char *comment);

StackContext createCodeGenContext(const char *file);

void freeCodegenContext(StackContext context);

int allocateVariable(StackContext context, const char *name, DataType type);

StackVariable findStackVariable(StackContext context, const char *name);

void emitPreamble(StackContext context);

void generateLoadVariable(StackContext context, const char *varName, RegisterId reg);

void generateStoreVariable(StackContext context, const char *varName, RegisterId reg);

void generateLoadImmediate(StackContext context, const char *value, DataType type, RegisterId reg);

void generateBinaryOp(StackContext context, NodeTypes opType, RegisterId leftReg, RegisterId rightReg,
                      RegisterId resultReg, DataType operandType, int invert);

void generateUnaryOp(StackContext context, NodeTypes opType, RegisterId operandReg, RegisterId resultReg,
                     DataType operandType);

RegisterId generateExpressionToRegister(ASTNode node, StackContext context, RegisterId preferredReg);

int generateConditional(ASTNode node, StackContext context);

int generateLoop(ASTNode node, StackContext context);

DataType getOperandType(ASTNode node, StackContext context);

void collectStringLiterals(ASTNode node, StackContext context);

int isLiteral(ASTNode node);

void spillRegisterToStack(StackContext context, RegisterId reg, DataType type);

void restoreRegisterFromStack(StackContext context, RegisterId reg, DataType type);

RegisterId getOppositeBranchRegister(RegisterId reg);

int generateBuiltinFunctionCall(ASTNode node, StackContext context);


#endif //CINTERPRETER_CODEGENERATION_H
