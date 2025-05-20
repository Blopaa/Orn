//
// Created by pablo on 13/03/2025.
//

#ifndef LEXER_H
#define LEXER_H

#define ASSIGNEMENT "="
#define PUNCTUATION ";"
#define INT_DEFINITION "int"
#define STRING_DEFINITION "string"
#define QUOTES "\""
#define SUM_OPERATOR "+"
#define SUB_OPERATOR "-"
#define MULTIPLY_OPERATOR "*"
#define DIVIDE_OPERATOR "/"
#include <stddef.h>

typedef enum {
    TokenAssignement,
    TokenLiteral,
    TokenIntDefinition,
    TokenStringDefinition,
    TokenPunctuation,
    TokenQuotes,
    TokenSum,
    TokenSub,
    TokenMult,
    TokenDiv,
    TokenString
} TokenType;

typedef struct {
    char *value;
    TokenType type;
} TokenMap;

static const char SpecialCharMap[] = {
    ';',
    '='
};

static const TokenMap tokenMapping[] = {
    {INT_DEFINITION, TokenIntDefinition},
    {STRING_DEFINITION, TokenStringDefinition},
    {ASSIGNEMENT, TokenAssignement},
    {PUNCTUATION, TokenPunctuation},
    {QUOTES, TokenQuotes},
    {SUM_OPERATOR, TokenSum},
    {SUB_OPERATOR, TokenSub},
    {MULTIPLY_OPERATOR, TokenMult},
    {DIVIDE_OPERATOR, TokenDiv},
    {NULL, TokenLiteral}
};

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
