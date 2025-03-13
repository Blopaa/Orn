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
char ** tokenization(char * input) {
    int tokenCount = 1; // At least one token if input is not empty
    for (int i = 0; input[i] != '\0'; i++) {
        if (isspace(input[i]) || input[i] == '\n') {
            tokenCount++;
        }
    }
    char ** tokens = malloc((tokenCount + 1) * sizeof(char*));
    char ** tokensPtr = tokens;
    int i = 0;
    int j = 0;
    char * buffer = malloc(100 * sizeof(char));
    while(input[i] != '\0') {
        if(input[i] == '\n' || isspace(input[i])) {
            buffer[j] = '\0';
            *tokens = buffer;
            tokens++;
            buffer = malloc(100 * sizeof(char));
            j = 0;
        }else {
            buffer[j] = input[i];
            j++;
        }
        i++;
    }
    // Handle the last token if it exists
    if (j > 0) {
        buffer[j] = '\0';
        *tokens = buffer;
        tokens++;
    }
    *tokens = NULL;
    return tokensPtr;
}
