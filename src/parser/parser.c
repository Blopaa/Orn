//
// Created by pablo on 20/05/2025.
//

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int isStringLit(char* val) {
    return val != NULL && val[0] == '"' && val[strlen(val) - 1] == '"';
}

int isIntLit(char* val) {
    if (val == NULL) return 0;
    for (int i = 0; val[i] != '\0'; i++) {
        if (!isdigit(val[i])) return 0;
    }
    return 1;
}

// generates AST
ASTNode ASTGenerator(Token token) {
    ASTNode programNode = createNode("PROGRAM", null_NODE);
    ASTNode crrntStat = NULL; // current statement
    ASTNode ls = NULL; // last statement

    while (token && token->next != NULL) {
        token = token->next;

        NodeTypes type = getDecType(token->type);
        if (type != null_NODE) {
            crrntStat = createNode(NULL, type);
            if (token->next != NULL) {
                token = token->next;
                crrntStat->value = strdup(token->value);
                if (token ->next != NULL && token->next->type == TokenAssignement) {
                    token = token->next; // skip "=", future updates will take care of += -=;
                    if (token->next!=NULL) {
                        token = token->next;
                        ASTNode valNod = NULL;
                        if (isStringLit(token->value)) {
                            valNod = createNode(token->value, STRING_LIT);
                        }else if (isIntLit(token->value)) {
                            valNod = createNode(token->value, INT_LIT);
                        }else {
                            valNod = createNode(token->value, STRING_LIT);
                        }
                        crrntStat->children = valNod;
                    }
                }
            }
            if (programNode->children == NULL) {
                programNode->children = crrntStat;
            }else {
                ls->brothers = crrntStat;
            }
            ls = crrntStat;
        }
    }
    return programNode;
}

void printAST(ASTNode node, int depth) {
    if(node == NULL) return;

    for(int i = 0; i < depth; i++) {
        printf("  ");
    }

    char* nodeTypeStr;
    switch(node->NodeType) {
        case STRING_VARIABLE_DEFINITION: nodeTypeStr = "STRING_VAR_DEF"; break;
        case INT_VARIABLE_DEFINITION: nodeTypeStr = "INT_VAR_DEF"; break;
        case STRING_LIT: nodeTypeStr = "STRING_LIT"; break;
        case INT_LIT: nodeTypeStr = "INT_LIT"; break;
        case ADD_OP: nodeTypeStr = "ADD_OP"; break;
        case SUB_OP: nodeTypeStr = "SUB_OP"; break;
        default: nodeTypeStr = "PROGRAM"; break;
    }

    printf("%s", nodeTypeStr);
    if(node->value) {
        printf(": %s", node->value);
    }
    printf("\n");

    printAST(node->children, depth + 1);
    printAST(node->brothers, depth);
}

void freeAST(ASTNode node) {
    if(node == NULL) return;

    freeAST(node->children);
    freeAST(node->brothers);

    if(node->value) {
        free(node->value);
    }
    free(node);
}