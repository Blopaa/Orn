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


struct Token {
    TokenType type;
    char * value;
    struct Token * next;
};

typedef struct Token * TokenPtr;

char ** tokenization(char * input);

#endif //LEXER_H