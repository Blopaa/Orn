#include <stdio.h>
#include <string.h>

#include "errorHandling/errorHandling.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "testing/testing.h"

void printTokens(char **tokens) {
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
    while (current && current->next != NULL) {
        current = current->next;
        printf("Token %d: '%s', tipe: %d\n", i, current->value, current->type);
        i++;
    }
    printf("Total tokens proccesed: %d\n", i);
}

int main(int argc, char *argv[]) {

    if (argc == 2 && strcmp(argv[1], "--test") == 0) {
        runTests();
        return 0;
    }

    printf("=== LEXER TEST ===\n");
    char *input = "int num = 2 * 2;";
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

    printErrorSummary();

    printf("\nTip: Use './compiler --test' to run tests\n");

    return hasErrors() ? 1 : 0;
}