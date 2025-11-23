#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errorHandling.h"
#include "typeChecker.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "IR/ir.h"
#include "IR/optimization.h"

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
    printf("    %s --verbose <INPUT_FILE>       Show both IR, AST and compilation steps\n", programName);
    printf("    %s --ir <INPUT_FILE>            Show IR only\n", programName);
    printf("    %s --ast <INPUT_FILE>           Show AST only\n", programName);
    printf("    %s --help\n\n", programName);
    printf("OPTIONS:\n");
    printf("    --verbose    Enable verbose compilation output (shows --ir and --ast)\n");
    printf("    --ir         Show intermediate representation (TAC)\n");
    printf("    --ast        Show Abstract Syntax Tree\n");
    printf("    --help       Show this help message\n\n");
    printf("EXAMPLES:\n");
    printf("    %s program.orn\n", programName);
    printf("    %s --ir program.orn\n", programName);
    printf("    %s --ast program.orn\n", programName);
    printf("    %s --verbose program.orn\n\n", programName);
    
}

int main(int argc, char* argv[]) {
    const char* inputFile = NULL;
    int showIr = 0;
    int showAST = 0;
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
        showIr = 1;
        showAST = 1;
        inputFile = argv[2];
    } else if(argc >= 3 && strcmp(argv[1], "--ir")==0){
        showIr = 1;
        inputFile = argv[2];
    }else if(argc >= 3 && strcmp(argv[1], "--ast")==0){
        showAST = 1;
        inputFile = argv[2];
    }else {
        inputFile = argv[1];
    }

    if (showAST && showIr) {
        printf("=== COMPILER ===\n");
        printf("Input file: %s\n\n", inputFile);
    }

    char* input = readFile(inputFile);
    if (input == NULL) {
        return 1;
    }

    if (showIr && showAST) {
        printf("Source code:\n");
        printf("----------------------------------------\n");
        printf("%s\n", input);
        printf("----------------------------------------\n\n");
    }

    if (showIr && showAST) printf("1. LEXICAL ANALYSIS: ");
    TokenList* tokens = lex(input, inputFile);
    if (!tokens || hasErrors()) {
        if (showIr && showAST) printf("FAILED\n");
        printErrorSummary();
        if (tokens) freeTokens(tokens);
        free(input);
        return 1;
    }
    if (showIr && showAST) printf("OK (%zu tokens)\n", tokens->count);

    if (showIr && showAST) printf("2. PARSING: ");
    ASTContext *astContext = ASTGenerator(tokens);
    if (!astContext || !astContext->root || hasErrors()) {
        if (showIr && showAST) printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        if (astContext) freeASTContext(astContext);
        free(input);
        return 1;
    }
    if (showIr && showAST) printf("OK\n");

    if (showAST) {
        printAST(astContext->root, 0);
        printf("\n");
    }

    if (showIr && showAST) printf("3. TYPE CHECKING: ");
    TypeCheckContext globalSymbolTable = typeCheckAST(astContext->root, input, inputFile);
    if (!globalSymbolTable || hasErrors()) {
        if (showIr && showAST) printf("FAILED\n");
        printErrorSummary();
        freeTokens(tokens);
        freeASTContext(astContext);
        free(input);
        return 1;
    }
    if (showIr && showAST) printf("OK\n");

    if (showIr && showAST) printf("4. IR GENERATION: ");
    IrContext *ir = generateIr(astContext->root, globalSymbolTable);
    if (!ir) {
        if (showIr && showAST) printf("FAILED\n");
        fprintf(stderr, "Error: Failed to generate intermediate representation\n");
        freeTokens(tokens);
        freeASTContext(astContext);
        freeTypeCheckContext(globalSymbolTable);
        free(input);
        return 1;
    }
    if (showIr && showAST) printf("OK (%d instructions)\n", ir->instructionCount);

    if (showIr) {
        constantFolding(ir);
        copyProp(ir);
        printIR(ir);
    }

    printErrorSummary();
    if (showIr && showAST) {
        printf("OK → %s\n\n", outputFile);
        printf("✓ Compilation SUCCESSFUL\n");
        printf("IR: %d instructions, %d temporaries, %d labels\n", ir->instructionCount,
               ir->nextTempNum - 1, ir->nextLabelNum - 1);
    } else {
        printf("Successfully compiled '%s' → '%s'\n", inputFile, outputFile);
    }

    freeTokens(tokens);
    freeASTContext(astContext);
    free(input);

    return 0;
}