#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errorHandling.h"
#include "typeChecker.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codeGeneration/codegen.h"
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
    printf("    %s <INPUT_FILE>                  Compile to executable\n", programName);
    printf("    %s -S <OUTPUT> <INPUT_FILE>      Output assembly only\n", programName);
    printf("    %s -o <OUTPUT> <INPUT_FILE>      Specify output file\n", programName);
    printf("    %s --ir <INPUT_FILE>             Show IR only\n", programName);
    printf("    %s --ast <INPUT_FILE>            Show AST only\n", programName);
    printf("    %s --verbose <INPUT_FILE>        Show all stages\n", programName);
    printf("    %s -O<level> <INPUT_FILE>        Set optimization level (0-3)\n", programName);
    printf("    %s --help                        Show this help\n\n", programName);
    printf("OPTIONS:\n");
    printf("    -S           Output assembly file (.s) instead of executable\n");
    printf("    -o <file>    Write output to <file>\n");
    printf("    --verbose    Show AST, IR, and compilation steps\n");
    printf("    --ir         Show intermediate representation (TAC)\n");
    printf("    --ast        Show Abstract Syntax Tree\n");
    printf("    -O0          No optimization (default)\n");
    printf("    -O1          Basic optimization (3 passes)\n");
    printf("    -O2          Moderate optimization (5 passes)\n");
    printf("    -O3          Aggressive optimization (10 passes)\n");
    printf("    --help       Show this help message\n\n");
    printf("EXAMPLES:\n");
    printf("    %s program.orn                   Compile to ./program\n", programName);
    printf("    %s -S program.orn                Output program.s\n", programName);
    printf("    %s -O2 -o myapp program.orn      Optimize and output to myapp\n", programName);
}

int main(int argc, char* argv[]) {
    const char* inputFile = NULL;
    const char* outputFile = NULL;
    int showIr = 0;
    int showAST = 0;
    int asmOnly = 0;
    int optLvl = 0;

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "--verbose") == 0) {
            showIr = 1;
            showAST = 1;
        }
        else if (strcmp(argv[i], "--ir") == 0) {
            showIr = 1;
        }
        else if (strcmp(argv[i], "--ast") == 0) {
            showAST = 1;
        }
        else if (strcmp(argv[i], "-S") == 0) {
            asmOnly = 1;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                outputFile = argv[++i];
            }
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requires an argument\n");
                return 1;
            }
        }
        else if (strncmp(argv[i], "-O", 2) == 0) {
            char level = argv[i][2];
            if (level >= '0' && level <= '3') {
                optLvl = level - '0';
            } else {
                fprintf(stderr, "Invalid optimization level: %s (use -O0 to -O3)\n", argv[i]);
                return 1;
            }
        }
        else if (argv[i][0] != '-') {
            inputFile = argv[i];
        }
        else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (!inputFile) {
        fprintf(stderr, "Error: No input file specified\n");
        printUsage(argv[0]);
        return 1;
    }

    // Determine output filenames
    char asmFile[256];
    char exeFile[256];
    
    if (outputFile) {
        if (asmOnly) {
            snprintf(asmFile, sizeof(asmFile), "%s", outputFile);
        } else {
            snprintf(asmFile, sizeof(asmFile), "%s.s", outputFile);
            snprintf(exeFile, sizeof(exeFile), "%s", outputFile);
        }
    } else {
        // Derive from input filename
        const char *dot = strrchr(inputFile, '.');
        const char *slash = strrchr(inputFile, '/');
        const char *baseName = slash ? slash + 1 : inputFile;
        size_t baseLen = dot && dot > baseName ? (size_t)(dot - baseName) : strlen(baseName);
        
        snprintf(asmFile, sizeof(asmFile), "%.*s.s", (int)baseLen, baseName);
        snprintf(exeFile, sizeof(exeFile), "%.*s", (int)baseLen, baseName);
    }

    // Verbose header
    if (showAST && showIr) {
        printf("=== ORN COMPILER ===\n");
        printf("Input file: %s\n", inputFile);
        printf("Optimization level: -O%d\n\n", optLvl);
    }

    // Read source file
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

    // === LEXICAL ANALYSIS ===
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

    // === PARSING ===
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
        printf("\n=== AST ===\n");
        printAST(astContext->root, 0);
        printf("\n");
    }

    // === TYPE CHECKING ===
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

    // === IR GENERATION ===
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

    if (optLvl > 0) {
        if (showIr && showAST) printf("5. OPTIMIZATION (-O%d): ", optLvl);
        optimizeIR(ir, optLvl);
        if (showIr && showAST) printf("OK (%d instructions after optimization)\n", ir->instructionCount);
    }

    if (showIr) {
        printf("\n=== IR (Three-Address Code) ===\n");
        printIR(ir);
        printf("\n");
    }

    if (showIr && showAST) printf("%d. CODE GENERATION: ", optLvl > 0 ? 6 : 5);
    
    char *assembly = generateAssembly(ir);
    if (!assembly) {
        if (showIr && showAST) printf("FAILED\n");
        fprintf(stderr, "Error: Failed to generate assembly\n");
        freeTokens(tokens);
        freeASTContext(astContext);
        freeTypeCheckContext(globalSymbolTable);
        freeIrContext(ir);
        free(input);
        return 1;
    }
    if (showIr && showAST) printf("OK\n");

    if (!writeAssemblyToFile(assembly, asmFile)) {
        fprintf(stderr, "Error: Failed to write assembly to '%s'\n", asmFile);
        free(assembly);
        freeTokens(tokens);
        freeASTContext(astContext);
        freeTypeCheckContext(globalSymbolTable);
        freeIrContext(ir);
        free(input);
        return 1;
    }

    if (showIr && showAST) {
        printf("\n=== GENERATED ASSEMBLY ===\n");
        printf("%s\n", assembly);
    }

    if (!asmOnly) {
        if (showIr && showAST) printf("%d. ASSEMBLING & LINKING: ", optLvl > 0 ? 7 : 6);
        
        size_t cmdLen =
            snprintf(NULL, 0, "gcc -no-pie -nostdlib -o %s %s ./runtime.s 2>&1", exeFile, asmFile) +
            1;
        char *cmd = malloc(cmdLen);
        if (!cmd) {
            fprintf(stderr, "Error: Failed to allocate memory for command\n");
            free(assembly);
            freeTokens(tokens);
            freeASTContext(astContext);
            freeTypeCheckContext(globalSymbolTable);
            freeIrContext(ir);
            free(input);
            return 1;
        }
        snprintf(cmd, cmdLen, "gcc -no-pie -nostdlib -o %s %s ./runtime.s 2>&1", exeFile, asmFile);

        int result = system(cmd);
        free(cmd);
        if (result != 0) {
            if (showIr && showAST) printf("FAILED\n");
            fprintf(stderr, "Error: Assembly/linking failed\n");
            free(assembly);
            freeTokens(tokens);
            freeASTContext(astContext);
            freeTypeCheckContext(globalSymbolTable);
            freeIrContext(ir);
            free(input);
            return 1;
        }
        if (showIr && showAST) printf("OK\n");
        
        remove(asmFile);
    }

    // === SUCCESS ===
    printErrorSummary();
    
    if (showIr && showAST) {
        printf("\n✓ Compilation SUCCESSFUL\n");
        printf("  IR: %d instructions, %d temporaries, %d labels\n", 
               ir->instructionCount, ir->nextTempNum - 1, ir->nextLabelNum - 1);
        if (asmOnly) {
            printf("  Output: %s\n", asmFile);
        } else {
            printf("  Output: %s\n", exeFile);
        }
    } else {
        if (asmOnly) {
            printf("Compiled '%s' -> '%s'\n", inputFile, asmFile);
        } else {
            printf("Compiled '%s' -> '%s'\n", inputFile, exeFile);
        }
    }

    // Cleanup
    free(assembly);
    freeTokens(tokens);
    freeASTContext(astContext);
    freeTypeCheckContext(globalSymbolTable);
    freeIrContext(ir);
    free(input);

    return 0;
}