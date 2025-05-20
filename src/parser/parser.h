//
// Created by pablo on 20/05/2025.
//

#include "../lexer/lexer.h"

#ifndef PARSER_H
#define PARSER_H

typedef enum {
     STRING_VARIABLE_DEFINITION,
     INT_VARIABLE_DEFINITION,
     ADD_OP,
     SUB_OP,
     INT_LIT,
     STRING_LIT,
     null_NODE
} NodeTypes ;

typedef struct {
     TokenType TkType;
     NodeTypes type;
} NodeMap;


static const NodeMap TypeDefs[] = {
     {TokenStringDefinition, STRING_VARIABLE_DEFINITION},
     {TokenIntDefinition, INT_VARIABLE_DEFINITION},
     {TokenNULL, null_NODE}
};


struct ASTNode {
     char * value;
     NodeTypes NodeType;
     struct ASTNode * children;
     struct ASTNode * brothers;
};

typedef struct ASTNode * ASTNode;

#endif //PARSER_H
