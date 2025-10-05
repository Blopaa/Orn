//
// Created by pablo on 15/09/2025.
//

#ifndef CINTERPRETER_SEMANTICHELPERS_H
#define CINTERPRETER_SEMANTICHELPERS_H

#include "parser.h"
#include "errorHandling.h"
#include "typeChecker.h"

ErrorContext *createErrorContextFromType(ASTNode node, TypeCheckContext context);
void freeErrorContext(ErrorContext * ctx);

#endif //CINTERPRETER_SEMANTICHELPERS_H