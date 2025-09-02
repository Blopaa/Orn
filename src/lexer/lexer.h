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
    TokenString,
    TokenPlusAssign, // +=
    TokenSubAssign, // -=
    TokenMultAssign, // *=
    TokenDivAssign, // /=

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

#endif //LEXER_H
