#include <stdio.h>
#include "lexer/lexer.h"

void printTokens(char** tokens) {
    if (tokens == NULL) {
        printf("No tokens found.\n");
        return;
    }

    int i = 0;
    while (tokens[i] != NULL) {
        printf("Token %d: '%s'\n", i, tokens[i]);
        i++;
    }
    printf("Total tokens: %d\n", i);
}

int main(void) {
    char * input = "int x = 5 ;";
    char ** res = tokenization(input);
    printTokens(res);
    return 0;
}
