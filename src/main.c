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
        printf("Token %d: '%s', tipo: %d\n", i, current->value, current->type);
        i++;
    }
    printf("Total tokens procesados: %d\n", i);
}

int main(void) {
    printf("=== LEXER TEST ===\n");
    char * input = "string sum = \"hola\";";
    printf("Input: %s\n\n", input);

    // Fase 1: Splitting
    printf("1. SPLITTING:\n");
    Input res = splitter(input);
    printTokens(res->input);
    printf("\n");

    // Fase 2: Tokenization
    printf("2. TOKENIZATION:\n");
    Token t = tokenization(res);
    printTokenList(t);
    printf("\n");

    // Fase 3: AST Generation
    printf("3. AST GENERATION:\n");
    ASTNode ast = ASTGenerator(t);
    printAST(ast, 0);
    printf("\n");

    // Limpiar memoria
    freeAST(ast);

    printf("=== EXAMPLES ===\n");
    char* examples[] = {
        "string name = \"Pablo\";",
        "int age = 25;",
        NULL
    };

    for(int i = 0; examples[i] != NULL; i++) {
        printf("\nInput: %s\n", examples[i]);
        Input inp = splitter(examples[i]);
        Token tokens = tokenization(inp);
        ASTNode tree = ASTGenerator(tokens);
        printAST(tree, 0);
        freeAST(tree);
    }

    return 0;
}