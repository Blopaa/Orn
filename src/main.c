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
    char * input = "string x = \"hello world\" ;";
    Input res = splitter(input);
    printTokens(res->input);
    Token t = tokenization(res);
    int i = 0;
    while(t->next != NULL) {
        t = t->next;
        printf("Token %d: '%s, tipo: %d'\n", i, t->value, t->type);
        i++;
    }
    return 0;
}
