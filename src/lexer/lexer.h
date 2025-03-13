//
// Created by pablo on 13/03/2025.
//

#ifndef LEXER_H
#define LEXER_H

struct Token {
    int type;
    char * value;
    struct Token * next;
};

typedef struct Token * TokenPtr;

#endif //LEXER_H
