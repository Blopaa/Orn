//
// Created by pablo on 20/05/2025.
//

#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"

// Enum for the different types of nodes in the AST
typedef enum {
    null_NODE,
    PROGRAM,
    STRING_VARIABLE_DEFINITION,
    INT_VARIABLE_DEFINITION,
    FLOAT_VARIABLE_DEFINITION,
    BOOL_VARIABLE_DEFINITION,
    STRING_LIT,
    INT_LIT,
    FLOAT_LIT,
    BOOL_LIT,
    VARIABLE,
    ASSIGNMENT,
    COMPOUND_ADD_ASSIGN,
    COMPOUND_SUB_ASSIGN,
    COMPOUND_MUL_ASSIGN,
    COMPOUND_DIV_ASSIGN,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    MOD_OP,
    UNARY_MINUS_OP,
    PRE_INCREMENT,
    PRE_DECREMENT,
    POST_INCREMENT,
    POST_DECREMENT,
    LOGIC_AND,
    LOGIC_OR,
    LOGIC_NOT,
    EQUAL_OP,
    NOT_EQUAL_OP,
    LESS_THAN_OP,
    GREATER_THAN_OP,
    LESS_EQUAL_OP,
    GREATER_EQUAL_OP,
    BLOCK_STATEMENT
} NodeTypes;

// Structure for an Abstract Syntax Tree node
struct ASTNode {
    char *value;
    NodeTypes NodeType;
    int line;
    int column;
    struct ASTNode *children;
    struct ASTNode *brothers;
};

typedef struct ASTNode *ASTNode;

// Struct to map token types to node types for declarations
typedef struct {
    TokenType TkType;
    NodeTypes type;
} TypeDefMap;

static const TypeDefMap TypeDefs[] = {
    {TokenIntDefinition, INT_VARIABLE_DEFINITION},
    {TokenStringDefinition, STRING_VARIABLE_DEFINITION},
    {TokenFloatDefinition, FLOAT_VARIABLE_DEFINITION},
    {TokenBoolDefinition, BOOL_VARIABLE_DEFINITION},
    {TokenNULL, null_NODE}
};

// Operator precedence for Pratt parser
typedef enum {
    PREC_NONE,
    PREC_ASSIGN,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY
} Precedence;

typedef struct {
    TokenType token;
    NodeTypes nodeType;
    Precedence precedence;
    int isRightAssociative; // Flag for right-associative operators like '='
} OperatorInfo;

static const OperatorInfo operators[] = {
    {TokenAssignement, ASSIGNMENT, PREC_ASSIGN, 1},
    {TokenPlusAssign, COMPOUND_ADD_ASSIGN, PREC_ASSIGN, 1},
    {TokenSubAssign, COMPOUND_SUB_ASSIGN, PREC_ASSIGN, 1},
    {TokenMultAssign, COMPOUND_MUL_ASSIGN, PREC_ASSIGN, 1},
    {TokenDivAssign, COMPOUND_DIV_ASSIGN, PREC_ASSIGN, 1},
    {TokenOr, LOGIC_OR, PREC_OR, 0},
    {TokenAnd, LOGIC_AND, PREC_AND, 0},
    {TokenEqual, EQUAL_OP, PREC_EQUALITY, 0},
    {TokenNotEqual, NOT_EQUAL_OP, PREC_EQUALITY, 0},
    {TokenLess, LESS_THAN_OP, PREC_COMPARISON, 0},
    {TokenGreater, GREATER_THAN_OP, PREC_COMPARISON, 0},
    {TokenLessEqual, LESS_EQUAL_OP, PREC_COMPARISON, 0},
    {TokenGreaterEqual, GREATER_EQUAL_OP, PREC_COMPARISON, 0},
    {TokenSum, ADD_OP, PREC_TERM, 0},
    {TokenSub, SUB_OP, PREC_TERM, 0},
    {TokenMult, MUL_OP, PREC_FACTOR, 0},
    {TokenDiv, DIV_OP, PREC_FACTOR, 0},
    {TokenMod, MOD_OP, PREC_FACTOR, 0},
    {TokenNULL, null_NODE, PREC_NONE, 0}
};


// Public function prototypes
ASTNode ASTGenerator(Token token);
void printAST(ASTNode node, int depth);
void freeAST(ASTNode node);

#endif //PARSER_H
