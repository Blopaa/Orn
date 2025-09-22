//
// Created by pablo on 22/09/2025.
//

#ifndef CINTERPRETER_CODEGENOPERATIONS_H
#define CINTERPRETER_CODEGENOPERATIONS_H
#include "codeGeneration.h"

void generateBinaryOp(StackContext context, NodeTypes opType,
                      RegisterId leftReg, RegisterId rightReg,
                      RegisterId resultReg, DataType operandType, int invert);
void generateUnaryOp(StackContext context, NodeTypes opType,
                     RegisterId operandReg, RegisterId resultReg,
                     DataType operandType);
void generateFloatBinaryOp(StackContext context, NodeTypes opType,
                           RegisterId leftReg, RegisterId rightReg,
                           RegisterId resultReg);
void generateFloatUnaryOp(StackContext context, NodeTypes opType,
                          RegisterId operandReg, RegisterId resultReg);
void generateStringOperation(StackContext context, NodeTypes opType,
                             RegisterId leftReg, RegisterId rightReg,
                             RegisterId resultReg);

#endif // CINTERPRETER_CODEGENOPERATIONS_H
