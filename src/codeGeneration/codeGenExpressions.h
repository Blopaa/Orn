//
// Created by pablo on 22/09/2025.
//

#ifndef CINTERPRETER_CODEGENEXPRESSIONS_H
#define CINTERPRETER_CODEGENEXPRESSIONS_H
#include "codeGeneration.h"

RegisterId generateExpressionToRegister(ASTNode node, StackContext context,
                                        RegisterId preferredReg);
int generateConditional(ASTNode node, StackContext context);
int generateLoop(ASTNode node, StackContext context);
int generateBuiltinFunctionCall(ASTNode node, StackContext context);

#endif // CINTERPRETER_CODEGENEXPRESSIONS_H
