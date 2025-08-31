#include <stdio.h>

#include "errorHandling/errorHandling.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

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

int main(void) {
    printf("=== LEXER TEST ===\n");
    char *input = "int c = 4.3;";
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

    // char *testInputs[] = {
    //     "string name = \"Pablo\";",
    //     "int age = 25;",
    //     "string errorTest1 = 123;",
    //     "int errorTest2 = \"hello\";",
    //     "int sum = a + b;",
    //     NULL
    // };
    //
    // for (int i = 0; testInputs[i] != NULL; i++) {
    //     printf("\n--- Testing: %s ---\n", testInputs[i]);
    //
    //     Input inp = splitter(testInputs[i]);
    //     Token tokens = tokenization(inp);
    //     ASTNode tree = ASTGenerator(tokens);
    //
    //     if (tree && !hasErrors()) {
    //         printAST(tree, 0);
    //     }
    //
    //     freeAST(tree);
    // }

    printErrorSummary();

    return hasErrors() ? 1 : 0;
}
