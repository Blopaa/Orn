#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errorHandling.h"
#include "typeChecker.h"
#include "codeGeneration/codeGeneration.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

char* readFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize == -1) {
        fprintf(stderr, "Error: Cannot determine size of file '%s'\n", filename);
        fclose(file);
        return NULL;
    }

    char* content = malloc(fileSize + 1);
    if (content == NULL) {
        fprintf(stderr, "Error: Cannot allocate memory for file '%s'\n", filename);
        fclose(file);
        return NULL;
    }
    size_t bytesRead = fread(content, 1, fileSize, file);
    content[bytesRead] = '\0';

    fclose(file);
    return content;
}

void printUsage(const char* programName) {
    printf("Orn Lang Compiler\n\n");
    printf("USAGE:\n");
    printf("    %s <INPUT_FILE>\n", programName);
    printf("    %s --verbose <INPUT_FILE>\n", programName);
    printf("    %s --help\n\n", programName);
    printf("OPTIONS:\n");
    printf("    --verbose    Enable verbose compilation output\n");
    printf("    --help       Show this help message\n\n");
    printf("EXAMPLES:\n");
    printf("    %s program.orn\n", programName);
    printf("    %s --verbose program.orn\n\n", programName);
}

int main(int argc, char* argv[]) {
    const char* inputFile = NULL;
    int verbose = 0;
    const char* outputFile = "output.s";

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        printUsage(argv[0]);
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "--verbose") == 0) {
        verbose = 1;
        inputFile = argv[2];
    } else {
        inputFile = argv[1];
    }

    if (verbose) {
        printf("=== COMPILER ===\n");
        printf("Input file: %s\n\n", inputFile);
    }

    char* input = readFile(inputFile);
    if (input == NULL) {
        return 1;
    }

    if (verbose) {
        printf("Input:\n%s\n\n", input);
    }

    if (verbose) printf("1. LEXICAL ANALYSIS: ");
    TokenList* tokens = lex(input, inputFile);
    if (!tokens || hasErrors()) {
        if (verbose) printf("FAILED\n");
        printErrorSummary();
        if (tokens) freeTokens(tokens);
        free(input);
        return 1;
    }
    if (verbose) printf("OK (%zu tokens)\n", tokens->count);

    if (verbose) printf("2. PARSING: ");
    ASTContext *astContext = ASTGenerator(tokens);
    if (!astContext || !astContext->root || hasErrors()) {
        if (verbose) printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        if (astContext) freeASTContext(astContext);
        free(input);
        return 1;
    }
    if (verbose) printf("OK\n");

    if (verbose) {
        printAST(astContext->root, 0);
        printf("\n");
    }

    if (verbose) printf("3. TYPE CHECKING: ");
    int typeCheckSuccess = typeCheckAST(astContext->root, input, inputFile);
    if (!typeCheckSuccess) {
        if (verbose) printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        freeASTContext(astContext);
        free(input);
        return 1;
    }
    if (verbose) printf("OK\n");

    if (verbose) printf("4. CODE GENERATION: ");
    int codeGenSuccess = generateCode(astContext->root, outputFile, input, inputFile);
    if (!codeGenSuccess) {
        if (verbose) printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        freeASTContext(astContext);
        free(input);
        return 1;
    }

    if (verbose) {
        printf("OK → %s\n\n", outputFile);
        printf("✓ Compilation SUCCESSFUL\n");
    } else {
        printf("Successfully compiled '%s' → '%s'\n", inputFile, outputFile);
    }

    freeTokens(tokens);
    freeASTContext(astContext);
    free(input);

    return 0;
}