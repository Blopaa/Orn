//
// Created by pablo on 08/09/2025.
//

#ifndef CINTERPRETER_TYPECHECKER_H
#define CINTERPRETER_TYPECHECKER_H
#include "symbolTable.h"
#include "../errorHandling/errorHandling.h"

/**
 * @brief Type checking context for managing symbol tables and scope.
 *
 * Maintains the current symbol table context for type checking operations.
 * Tracks both the global scope and current active scope to enable proper
 * variable resolution and scope management during AST traversal.
 */
typedef struct TypeCheckContext {
    SymbolTable current;
    SymbolTable global;
} *TypeCheckContext;

TypeCheckContext createTypeCheckContext();

void freeTypeCheckContext(TypeCheckContext context);

int areCompatible(DataType target, DataType source);

DataType getOperationResultType(DataType left, DataType right, NodeTypes op);

DataType getExpressionType(ASTNode node, TypeCheckContext context);

ErrorCode variableErrorCompatibleHandling(DataType varType, DataType initType);

int validateVariableDeclaration(ASTNode node, TypeCheckContext context);

int validateAssignment(ASTNode node, TypeCheckContext context);

int validateVariableUsage(ASTNode node, TypeCheckContext context);

int typeCheckNode(ASTNode node, TypeCheckContext context);

int typeCheckChildren(ASTNode node, TypeCheckContext context);

int typeCheckAST(ASTNode ast);

#endif //CINTERPRETER_TYPECHECKER_H
