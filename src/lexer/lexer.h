//
// Created by pablo on 13/03/2025.
//

#ifndef LEXER_H
#define LEXER_H

#define COMMENT "//"
#define ASSIGNEMENT "="
#define PUNCTUATION ";"
#define INT_DEFINITION "int"
#define STRING_DEFINITION "string"
#define FLOAT_DEFINITION "float"
#define BOOL_DEFINITION "bool"
#define TRUE_STATEMENT "true"
#define FALSE_STATEMENT "false"
#define QUOTES "\""
#define SUM_OPERATOR "+"
#define SUB_OPERATOR "-"
#define MULTIPLY_OPERATOR "*"
#define DIVIDE_OPERATOR "/"
#define MODULUS_OPERATOR "%"
#define PLUS_ASSIGN "+="
#define SUB_ASSIGN "-="
#define MULTIPLY_ASSIGN "*="
#define DIVIDE_ASSIGN "/="
#define INCREMENT_OPERATOR "++"
#define DECREMENT_OPERATOR "--"
#define LOGICAL_AND "&&"
#define LOGICAL_OR "||"
#define LOGICAL_NOT "!"
#define EQUAL_OPERATOR "=="
#define NOT_EQUAL_OPERATOR "!="
#define LESS_THAN_OPERATOR "<"
#define GREATER_THAN_OPERATOR ">"
#define LESS_EQUAL_OPERATOR "<="
#define GREATER_EQUAL_OPERATOR ">="
#define RIGHT_BRACE "}"
#define LEFT_BRACE "{"
#include <stddef.h>


// ammount of types of tokens there can be
typedef enum {
    TokenAssignement, // =
    TokenLiteral,
    TokenIntDefinition, // int
    TokenStringDefinition, // string
    TokenFloatDefinition, // float
    TokenBoolDefinition, // bool
    TokenPunctuation, // ;
    TokenQuotes, // "
    TokenTrue, // true
    TokenFalse, // false
    TokenSum, // +
    TokenSub, // -
    TokenMult, // *
    TokenDiv, // /
    TokenMod, // %
    TokenIncrement, // ++
    TokenDecrement, // --
    TokenAnd, // &&
    TokenOr, // ||
    TokenNot, // !
    TokenString,
    TokenPlusAssign, // +=
    TokenSubAssign, // -=
    TokenMultAssign, // *=
    TokenDivAssign, // /=
    TokenEqual, // ==
    TokenNotEqual, // !=
    TokenLess, // <
    TokenGreater, // >
    TokenLessEqual, // <=
    TokenGreaterEqual, // >=
    TokenLeftBrace, // {
    TokenRightBrace, // }

    TokenNULL
} TokenType;


// Mapping type to quick find types by key
typedef struct {
    char *value;
    TokenType type;
} TokenMap;


// reserved chars
static const char *SpecialCharMap[] = {
    PUNCTUATION,
    ASSIGNEMENT,
    SUM_OPERATOR,
    SUB_OPERATOR,
    MULTIPLY_OPERATOR,
    DIVIDE_OPERATOR,
    MODULUS_OPERATOR,
    LOGICAL_NOT,
    LESS_THAN_OPERATOR,
    GREATER_THAN_OPERATOR,
    LEFT_BRACE,
    RIGHT_BRACE,
    NULL
};

// Map by key to quick find types
static const TokenMap tokenMapping[] = {
    {INT_DEFINITION, TokenIntDefinition},
    {STRING_DEFINITION, TokenStringDefinition},
    {FLOAT_DEFINITION, TokenFloatDefinition},
    {BOOL_DEFINITION, TokenBoolDefinition},
    {ASSIGNEMENT, TokenAssignement},
    {PUNCTUATION, TokenPunctuation},
    {QUOTES, TokenQuotes},
    {TRUE_STATEMENT, TokenTrue},
    {FALSE_STATEMENT, TokenFalse},
    {SUM_OPERATOR, TokenSum},
    {SUB_OPERATOR, TokenSub},
    {MULTIPLY_OPERATOR, TokenMult},
    {MODULUS_OPERATOR, TokenMod},
    {DIVIDE_OPERATOR, TokenDiv},
    {PLUS_ASSIGN, TokenPlusAssign},
    {SUB_ASSIGN, TokenSubAssign},
    {MULTIPLY_ASSIGN, TokenMultAssign},
    {DIVIDE_ASSIGN, TokenDivAssign},
    {INCREMENT_OPERATOR, TokenIncrement},
    {DECREMENT_OPERATOR, TokenDecrement},
    {LOGICAL_AND, TokenAnd},
    {LOGICAL_OR, TokenOr},
    {LOGICAL_NOT, TokenNot},
    {EQUAL_OPERATOR, TokenEqual},
    {NOT_EQUAL_OPERATOR, TokenNotEqual},
    {LESS_THAN_OPERATOR, TokenLess},
    {GREATER_THAN_OPERATOR, TokenGreater},
    {LESS_EQUAL_OPERATOR, TokenLessEqual},
    {GREATER_EQUAL_OPERATOR, TokenGreaterEqual},
    {LEFT_BRACE, TokenLeftBrace},
    {RIGHT_BRACE, TokenRightBrace},
    {NULL, TokenLiteral}
};

// Input send by splitter, it is the splitted content and its legth
struct Input {
    char **input;
    int n;
};

typedef struct Input *Input;

struct Token {
    TokenType type;
    char *value;
    struct Token *next;
};

typedef struct Token *Token;

Input splitter(const char *input);

Token tokenization(Input in);

void freeInput(Input in);

void freeTokenList(Token token);

#endif //LEXER_H
