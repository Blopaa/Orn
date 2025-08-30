#include <stdio.h>
#include "lexer/lexer.h"
#include "parser/parser.h"

void printTokens(char** tokens) {
    if (tokens == NULL) {
        printf("No tokens found.\n");
        return;
    }

    int i = 0;
    while (tokens[i] != NULL) {
        printf("split %d: '%s'\n", i, tokens[i]);
        i++;
    }
    printf("Total tokens: %d\n", i);
}

void printTokenList(Token t) {
    if (t == NULL) {
        printf("No tokens found.\n");
        return;
    }

    int i = 0;
    Token current = t;
    while(current && current->next != NULL) {
        current = current->next;
        printf("Token %d: '%s', tipe: %d\n", i, current->value, current->type);
        i++;
    }
    printf("Total tokens proccesed: %d\n", i);
}

int main(void) {
    printf("=== LEXER TEST ===\n");
    char * input = "string c = \"hola\"";
    printf("Input: %s\n\n", input);

    printf("1. SPLITTING:\n");
    Input res = splitter(input);
    printTokens(res->input);
    printf("\n");

    printf("2. TOKENIZATION:\n");
    Token t = tokenization(res);
    printTokenList(t);
    printf("\n");

    printf("3. AST GENERATION:\n");
    ASTNode ast = ASTGenerator(t);
    printAST(ast, 0);
    printf("\n");

    freeAST(ast);

    return 0;
}