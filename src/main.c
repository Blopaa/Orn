#include <stdio.h>
#include <string.h>

#include "errorHandling.h"
#include "typeChecker.h"
#include "codeGeneration/codeGeneration.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

// Updated to work with the new Input struct.
void printTokens(Input in) {
    if (in == NULL) {
        printf("No tokens found.\n");
        return;
    }

    printf("Raw tokens from splitter:\n");
    for (int i = 0; i < in->n; i++) {
        InputToken *raw_token = &in->tokens[i];
        printf("  Split %d (L%d, C%d): '%s'\n", i, raw_token->line, raw_token->column, raw_token->value);
    }
    printf("Total raw tokens: %d\n", in->n);
}

void printCompilationStatus(int parseSuccess, int typeCheckSuccess, int codeGenSuccess) {
    printf("=== COMPILATION STATUS ===\n");

    if (parseSuccess && typeCheckSuccess && codeGenSuccess) {
        printf("COMPILATION SUCCESSFUL\n");
        printf("   - Parsing: PASSED\n");
        printf("   - Type checking: PASSED\n");
        printf("   - Code generation: PASSED\n");
    } else if (parseSuccess && typeCheckSuccess && !codeGenSuccess) {
        printf("COMPILATION FAILED\n");
        printf("   - Parsing: PASSED\n");
        printf("   - Type checking: PASSED\n");
        printf("   - Code generation: FAILED\n");
    } else if (parseSuccess && !typeCheckSuccess) {
        printf("COMPILATION FAILED\n");
        printf("   - Parsing: PASSED\n");
        printf("   - Type checking: FAILED\n");
        printf("   - Code generation: SKIPPED\n");
    } else {
        printf("COMPILATION FAILED\n");
        printf("   - Parsing: FAILED\n");
        printf("   - Type checking: SKIPPED\n");
        printf("   - Code generation: SKIPPED\n");
    }

    printf("\n");
}

void printTokenList(Token t) {
    if (t == NULL || t->next == NULL) {
        printf("No tokens found.\n");
        return;
    }

    int i = 0;
    // Start with t->next to skip the dummy head node.
    Token current = t->next;
    while (current != NULL) {
        printf("Token %d (L%d, C%d): '%s', type: %d\n",
               i, current->line, current->column, current->value, current->type);
        i++;
        current = current->next;
    }
    printf("Total tokens processed: %d\n", i);
}

int main() {
    printf("=== LEXER TEST ===\n");
    char *input = "int x = 10; print(x);";
    printf("Input:\n%s\n\n", input);

    // Use the new two-step lexer process
    printf("1. SPLITTING:\n");
    Input res = splitter(input);
    printTokens(res); // Call the updated helper function
    printf("\n");

    printf("2. TOKENIZATION:\n");
    // 'res' is consumed and freed by the tokenization function.
    Token t = tokenization(res);
    printTokenList(t);
    printf("\n");

    printf("3. AST GENERATION:\n");
    ASTNode ast = ASTGenerator(t);
    int parseSuccess = (ast != NULL && !hasErrors());
    printAST(ast, 0);
    printf("\n");

    printf("4. TYPE CHECKING:\n");
    int typeCheckSuccess = 0;

    if (parseSuccess && ast != NULL) {
        printf("   Running semantic analysis...\n");
        typeCheckSuccess = typeCheckAST(ast);

        if (typeCheckSuccess) {
            printf("   Type checking PASSED\n");
        } else {
            printf("   Type checking FAILED\n");
        }
    } else {
        printf("   Skipping type checking due to parse errors\n");
    }
    printf("\n");

    printf("5. CODE GENERATION:\n");
    int codeGenSuccess = 0;

    if (parseSuccess && typeCheckSuccess) {
        printf("   Generating assembly code...\n");
        const char *outputFile = "../output.s";
        codeGenSuccess = generateCode(ast, outputFile);

        if (codeGenSuccess) {
            printf("   Code generation PASSED\n");
            printf("   Output file: %s\n", outputFile);
        } else {
            printf("   Code generation FAILED\n");
        }
    } else {
        printf("   Skipping code generation due to prior errors\n");
    }
    printf("\n");

    printf("6. ERROR SUMMARY:\n");
    printErrorSummary();
    printf("\n");

    printCompilationStatus(parseSuccess, typeCheckSuccess, codeGenSuccess);

    // Free the final data structures.
    freeTokenList(t);
    freeAST(ast);

    printf("\nTip: Use './compiler --test' to run tests\n");

    return (parseSuccess && typeCheckSuccess && codeGenSuccess) ? 0 : 1;
}