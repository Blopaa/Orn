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
    Input res = splitter(input);
    printTokens(res->input);
    Token t = tokenization(res);
    for(int i = 0; i < res->n; i++) {
        t = t->next;
        printf("Token %d: '%s, tipo: %d'\n", i, t->value, t->type);
    }
    t->next == NULL ? printf("nulo") : printf("no nulo");
    return 0;
}
