#include <stdio.h>
#include <string.h>

#include "errorHandling.h"
#include "typeChecker.h"
#include "codeGeneration/codeGeneration.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

int main() {
    printf("=== COMPILER ===\n");
	const char* outputFile = "../output.s";
    const char* input = "int x = \"hi\";\n"
                        "bool y = 1;\n";

    printf("Input:\n%s\n\n", input);

    // Step 1: Lexical Analysis
    printf("1. LEXICAL ANALYSIS: ");
    TokenList* tokens = lex(input, outputFile);
    if (!tokens || hasErrors()) {
        printf("FAILED\n");
        printErrorSummary();
        if (tokens) freeTokens(tokens);
        return 1;
    }
    printf("OK (%zu tokens)\n", tokens->count);

    // Step 2: Parsing
    printf("2. PARSING: \n");
    ASTNode ast = ASTGenerator(tokens);
    if (!ast || hasErrors()) {
        printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        if (ast) freeAST(ast);
        return 1;
    }
    printf("OK\n");
    printAST(ast, 0);
    printf("\n");

    // Step 3: Type Checking
    printf("3. TYPE CHECKING: \n");
    int typeCheckSuccess = typeCheckAST(ast, input, outputFile);
    if (!typeCheckSuccess) {
        printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        freeAST(ast);
        return 1;
    }
    printf("OK\n");

    // Step 4: Code Generation
    printf("4. CODE GENERATION: ");
    int codeGenSuccess = generateCode(ast, outputFile);
    if (!codeGenSuccess) {
        printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        freeAST(ast);
        return 1;
    }
    printf("OK → %s\n\n", outputFile);

    printf("✓ Compilation SUCCESSFUL\n");

    // Cleanup
    freeTokens(tokens);
    freeAST(ast);

    return 0;
}