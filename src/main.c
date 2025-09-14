#include <stdio.h>
#include <string.h>

#include "errorHandling.h"
#include "typeChecker.h"
#include "codeGeneration/codeGeneration.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

int main() {
    printf("=== COMPILER ===\n");

    const char* input = "int x = 10;\n"
                       "string space = \" \";\n"
                       "while x <= 10 && x >= -10 {\n"
                       "    print(x);\n"
                       "    print(space);\n"
                       "    x--;\n"
                       "};";

    printf("Input:\n%s\n\n", input);

    // Step 1: Lexical Analysis
    printf("1. LEXICAL ANALYSIS: ");
    TokenList* tokens = lex(input);
    if (!tokens || hasErrors()) {
        printf("FAILED\n");
        printErrorSummary();
        if (tokens) freeTokens(tokens);
        return 1;
    }
    printf("OK (%zu tokens)\n", tokens->count);

    // Step 2: Parsing
    printf("2. PARSING: ");
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
    printf("3. TYPE CHECKING: ");
    int typeCheckSuccess = typeCheckAST(ast);
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
    const char* outputFile = "../output.s";
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