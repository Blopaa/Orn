//
// Created by pablo on 13/03/2025.
//

#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This function will take a string as input and return a linked list of tokens
// @param - input
Input splitter(char *input) {
    Input in = malloc(sizeof(struct Input));
    int tokenCount = 1;
    for (int i = 0; input[i] != '\0'; i++) {
        if (isspace(input[i]) || input[i] == '\n') {
            tokenCount++;
        }
    }
    in->n = tokenCount;
    char **tokens = malloc((tokenCount + 1) * sizeof(char *));
    in->input = tokens;
    int i = 0;
    int j = 0;
    char *buffer = malloc(100 * sizeof(char));
    while (input[i] != '\0') {
        if (input[i] == '\n' || isspace(input[i])) {
            buffer[j] = '\0';
            *tokens = buffer;
            tokens++;
            buffer = malloc(100 * sizeof(char));
            j = 0;
        } else {
            buffer[j] = input[i];
            j++;
        }
        i++;
    }
    if (j > 0) {
        buffer[j] = '\0';
        *tokens = buffer;
        tokens++;
    }
    *tokens = NULL;
    return in;
}

TokenType findTokenType(char *val) {
    for (int i = 0; tokenMapping[i].value != NULL; i++) {
        if (strcmp(tokenMapping[i].value, val) == 0) {
            return tokenMapping[i].type;
        }
    }
    return TokenLiteral;
}

char *getStringToken(char **val, int *n, int maxLength) {
    char *str = malloc(100 * sizeof(char));
    str[0] = '\0';
    while (*n < maxLength && (!strstr(val[*n], QUOTES) || findTokenType(val[*n]) != TokenQuotes)) {
        if(str[0] != '\0') strcat(str, " ");
        strcat(str, val[*n]);
        (*n)++;
    }
    return str;
}

Token tokenization(Input in) {
    Token head = malloc(sizeof(struct Token));
    Token crrnt = head;
    for (int i = 0; i < in->n; i++) {
        crrnt->next = malloc(sizeof(struct Token));
        crrnt = crrnt->next;

        crrnt->type = findTokenType(in->input[i]);
        if (crrnt->type == TokenQuotes || (strstr(in->input[i], QUOTES) && crrnt->type == TokenLiteral)) {
            crrnt->type = TokenString;
            crrnt->value = getStringToken(in->input, &i, in->n);
        } else {
            crrnt->value = in->input[i];
        }
    }
    crrnt->next = NULL;
    return head;
}
