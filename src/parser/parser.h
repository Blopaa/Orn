//
// Created by pablo on 20/05/2025.
//

#include "../lexer/lexer.h"

#ifndef PARSER_H
#define PARSER_H

typedef enum {
    STRING_VARIABLE_DEFINITION,
    INT_VARIABLE_DEFINITION,
    FLOAT_VARIABLE_DEFINITION,
    BOOL_VARIABLE_DEFINITION,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    MOD_OP,
    INT_LIT,
    FLOAT_LIT,
    STRING_LIT,
    BOOL_LIT,
    LOGIC_AND,
    LOGIC_OR,
    LOGIC_NOT,
    VARIABLE,
    EQUAL_OP,
    NOT_EQUAL_OP,
    LESS_THAN_OP,
    GREATER_THAN_OP,
    LESS_EQUAL_OP,
    GREATER_EQUAL_OP,
    UNARY_MINUS_OP,
    ASSIGNMENT,
    PRE_INCREMENT,
    PRE_DECREMENT,
    POST_INCREMENT,
    POST_DECREMENT,
    COMPOUND_ADD_ASSIGN,
    COMPOUND_SUB_ASSIGN,
    COMPOUND_MUL_ASSIGN,
    COMPOUND_DIV_ASSIGN,
    BLOCK_STATEMENT,
    null_NODE
} NodeTypes;

typedef enum {
    PREC_NONE,
    PREC_OR, // ||
    PREC_AND, // &&
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR, // * / %
    PREC_UNARY, // ! - ++ --
    PREC_PRIMARY // literals, variables
} Precedence;

typedef struct {
    TokenType token;
    NodeTypes nodeType;
    Precedence precedence;
} OperatorInfo;

static const OperatorInfo operators[] = {
    {TokenOr, LOGIC_OR, PREC_OR},
    {TokenAnd, LOGIC_AND, PREC_AND},
    {TokenEqual, EQUAL_OP, PREC_EQUALITY},
    {TokenNotEqual, NOT_EQUAL_OP, PREC_EQUALITY},
    {TokenLess, LESS_THAN_OP, PREC_COMPARISON},
    {TokenGreater, GREATER_THAN_OP, PREC_COMPARISON},
    {TokenLessEqual, LESS_EQUAL_OP, PREC_COMPARISON},
    {TokenGreaterEqual, GREATER_EQUAL_OP, PREC_COMPARISON},
    {TokenSum, ADD_OP, PREC_TERM},
    {TokenSub, SUB_OP, PREC_TERM},
    {TokenMult, MUL_OP, PREC_FACTOR},
    {TokenDiv, DIV_OP, PREC_FACTOR},
    {TokenMod, MOD_OP, PREC_FACTOR},
    {TokenNULL, null_NODE, PREC_NONE}
};


typedef struct {
    TokenType TkType;
    NodeTypes type;
} NodeMap;


static const NodeMap TypeDefs[] = {
    {TokenStringDefinition, STRING_VARIABLE_DEFINITION},
    {TokenIntDefinition, INT_VARIABLE_DEFINITION},
    {TokenFloatDefinition, FLOAT_VARIABLE_DEFINITION},
    {TokenBoolDefinition, BOOL_VARIABLE_DEFINITION},
    {TokenNULL, null_NODE}
};


struct ASTNode {
    char *value;
    NodeTypes NodeType;
    struct ASTNode *children;
    struct ASTNode *brothers;
};

typedef struct ASTNode *ASTNode;

NodeTypes getDecType(TokenType type);

ASTNode createNode(char *val, NodeTypes type);

ASTNode ASTGenerator(Token token);

void printAST(ASTNode node, int depth);

void freeAST(ASTNode node);

#endif //PARSER_H
