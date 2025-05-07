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

Token tokenization(Input in) {
    Token head = malloc(sizeof(struct Token));
    Token crrnt = head;
    for (int i = 0; i < in->n; i++) {
        crrnt->next = malloc(sizeof(struct Token));
        crrnt = crrnt->next;

        if(!strcmp(in->input[i],VALUE_INT_DEFINITION)) {
            crrnt->type = TokenDefinition;
            crrnt->value = VALUE_INT_DEFINITION;
            crrnt->next =  malloc(sizeof(struct Token));
            i++;
            crrnt = crrnt->next;
            crrnt->value = in->input[i];
            crrnt->type = TokenLiteral;
        }else if(!strcmp(in->input[i],VALUE_ASSIGNEMENT)) {
            crrnt->type = TokenAssignement;
            crrnt->value = in->input[i];
        }
        else if(!strcmp(in->input[i],VALUE_PUNCTUATION)) {
            crrnt->type = TokenPunctuation;
            crrnt->value = in->input[i];
        }
        else {
            crrnt->type = TokenLiteral;
            crrnt->value = in->input[i];
        }
    }
    crrnt->next = NULL;
    return head;
}
