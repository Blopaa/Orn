//
// Created by pablo on 13/03/2025.
//

#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// This function will take a string as input and return a linked list of tokens
// @param - input
Input splitter(const char *input) {
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

Token createToken(char *val, TokenType t) {
    Token tk = malloc(sizeof(struct Token));
    tk->type = t;
    tk->value = val;
    return tk;
}

TokenType findTokenType(const char *val) {
    for (int i = 0; tokenMapping[i].value != NULL; i++) {
        if (strcmp(tokenMapping[i].value, val) == 0) {
            return tokenMapping[i].type;
        }
    }
    return TokenLiteral;
}

char *getStringValue(char **val, int *n, int maxLength) {
    char *str = malloc(100 * sizeof(char));
    str[0] = '\0';
    strcat(str, val[*n]);
    (*n)++;
    while (*n < maxLength && (!strstr(val[*n - 1], QUOTES) || !strcmp(val[*n - 1], str))) {
        strcat(str, " ");
        strcat(str, val[*n]);
        (*n)++;
    }
    (*n)--;
    return str;
}

Token handleStringPunc (Token crrnt) {
    const size_t length = strlen(crrnt->value);
    if (!strcmp(&crrnt->value[length - 1], PUNCTUATION)) {
        crrnt->value[length - 1] = '\0';
        crrnt->next = createToken(PUNCTUATION, TokenPunctuation);
        crrnt = crrnt->next;
    }
    return crrnt;
}

Token tokenization(Input in) {
    struct Token * const head = malloc(sizeof(struct Token));
    Token crrnt = head;
    for (int i = 0; i < in->n; i++) {
        crrnt->next = malloc(sizeof(struct Token));
        crrnt = crrnt->next;

        crrnt->type = findTokenType(in->input[i]);
        if (crrnt->type == TokenQuotes || (strstr(in->input[i], QUOTES) && crrnt->type == TokenLiteral)) {
            crrnt->type = TokenString;
            crrnt->value = getStringValue(in->input, &i, in->n);
            crrnt = handleStringPunc(crrnt);
        } else {
            crrnt->value = in->input[i];
        }
    }
    crrnt->next = NULL;
    return head;
}
