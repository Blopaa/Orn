//
// Created by pablo on 20/05/2025.
//

#include "parser.h"

#include <stdlib.h>

#include "../lexer/lexer.h"

// returns the type of the variable declaration based on the type of the token given string | int
// will return null_NODE as sentinel type when its not a type of variableDec
NodeTypes getDecType(TokenType type) {
    for(int i = 0; TypeDefs[i].TkType != TokenNULL; i++) {
        if(TypeDefs[i].TkType == type) return TypeDefs[i].type;
    }
    return null_NODE;
}

ASTNode createNode(char* val, NodeTypes type) {
    ASTNode node = malloc(sizeof(struct ASTNode));
    node->value = val;
    node->NodeType = type;
    node->brothers = NULL;
    node->children = NULL;
    return node;
}

// generates AST
ASTNode ASTGenerator(Token token) {
    ASTNode programNode = malloc(sizeof(struct ASTNode));
    ASTNode crrnt = programNode;
    crrnt->children = malloc(sizeof(struct ASTNode));
    crrnt = crrnt->children;
    while(token->next != NULL) {
        token = token->next;
        NodeTypes decType = getDecType(token->type);
        if(decType != null_NODE) {
            crrnt->NodeType = decType;
            token->next;
            crrnt->value = token->value;
            token = token->next->next; // ignore =, usefull in the future to distinc += -=...
            // crrnt->children = createNode(token->value, )
        }
    }
}
