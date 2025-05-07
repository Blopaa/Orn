//
// Created by pablo on 13/03/2025.
//

#ifndef LEXER_H
#define LEXER_H

#define VALUE_ASSIGNEMENT "="
#define VALUE_PUNCTUATION ";"
#define VALUE_INT_DEFINITION "int"

typedef enum {
    TokenAssignement,
    TokenLiteral,
    TokenDefinition,
    TokenPunctuation,
} TokenType;

struct Input {
    char ** input;
    int n;
};

typedef struct Input * Input;

struct Token {
    TokenType type;
    char * value;
    struct Token * next;
};

typedef struct Token * Token;

Input splitter(char * input);

Token tokenization(Input input);

#endif //LEXER_H