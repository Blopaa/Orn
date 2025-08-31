//
// Created by pablo on 13/03/2025.
//

#ifndef LEXER_H
#define LEXER_H

#define ASSIGNEMENT "="
#define PUNCTUATION ";"
#define INT_DEFINITION "int"
#define STRING_DEFINITION "string"
#define FLOAT_DEFINITION "float"
#define QUOTES "\""
#define SUM_OPERATOR "+"
#define SUB_OPERATOR "-"
#define MULTIPLY_OPERATOR "*"
#define DIVIDE_OPERATOR "/"
#include <stddef.h>


// ammount of types of tokens there can be
typedef enum {
    TokenAssignement,
    TokenLiteral,
    TokenIntDefinition,
    TokenStringDefinition,
    TokenFloatDefinition,
    TokenPunctuation,
    TokenQuotes,
    TokenSum,
    TokenSub,
    TokenMult,
    TokenDiv,
    TokenString,
    TokenNULL
} TokenType;


// Mapping type to quick find types by key
typedef struct {
    char *value;
    TokenType type;
} TokenMap;


// reserved chars
static const char * SpecialCharMap[] = {
    PUNCTUATION,
    ASSIGNEMENT,
    SUM_OPERATOR,
    SUB_OPERATOR,
    MULTIPLY_OPERATOR,
    DIVIDE_OPERATOR,
    NULL
};

// Map by key to quick find types
static const TokenMap tokenMapping[] = {
    {INT_DEFINITION, TokenIntDefinition},
    {STRING_DEFINITION, TokenStringDefinition},
    {FLOAT_DEFINITION, TokenFloatDefinition},
    {ASSIGNEMENT, TokenAssignement},
    {PUNCTUATION, TokenPunctuation},
    {QUOTES, TokenQuotes},
    {SUM_OPERATOR, TokenSum},
    {SUB_OPERATOR, TokenSub},
    {MULTIPLY_OPERATOR, TokenMult},
    {DIVIDE_OPERATOR, TokenDiv},
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
