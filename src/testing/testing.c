#include "testing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../errorHandling/errorHandling.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"

static int testsRun = 0;
static int testsPassed = 0;

void assert(int condition, const char *testName) {
    testsRun++;
    if (condition) {
        testsPassed++;
        printf("✓ %s\n", testName);
    } else {
        printf("✗ %s\n", testName);
    }
}

void testTokenCount(const char *name, const char *input, int expectedTokens) {
    printf("\nTesting: %s\n", name);
    printf("Input: '%s'\n", input);
    printf("Expected tokens: %d\n", expectedTokens);

    resetErrorCount();

    Input res = splitter(input);

    printf("Actual tokens: %d\n", res->n);

    if (res->n > 0) {
        printf("Tokens found:\n");
        for (int i = 0; i < res->n; i++) {
            printf("  Token %d: '%s'\n", i, res->input[i]);
        }
    } else {
        printf("No tokens found.\n");
    }

    assert(res->n == expectedTokens, "Token count should match expected");

    for (int i = 0; i < res->n; i++) {
        free(res->input[i]);
    }
    free(res->input);
    free(res);

    printf("\n");
}

void testCase(const char *name, const char *input, int shouldPass) {
    printf("\nTesting: %s\n", name);
    printf("Input: %s\n", input);

    resetErrorCount();

    Input res = splitter(input);
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    if (shouldPass) {
        assert(ast != NULL, "AST should be generated");
        assert(!hasErrors(), "Should have no errors");
    } else {
        assert(hasErrors(), "Should have errors");
    }

    if (ast) {
        freeAST(ast);
    }

    printf("\n");
}

void testBasicCases(void) {
    printf("=== TESTING BASIC CASES ===\n");

    testCase("Basic int declaration", "int x = 5;", 1);
    testTokenCount("Basic int declaration", "int x = 5;", 5);
    testCase("Basic string declaration", "string name = \"Pablo\";", 1);
    testCase("Basic float declaration", "float pi = 3.14;", 1);
    testCase("Simple addition", "int sum = a + b;", 1);
    testTokenCount("Simple addition", "int sum = a + b;", 7);
    testCase("Simple subtraction", "int diff = x - y;", 1);
}

void testErrorCases(void) {
    printf("=== TESTING ERROR CASES ===\n");

    testCase("String to int error", "int x = \"hello\";", 0);
    testCase("Int to string error", "string text = 123;", 0);
    testCase("Float to string error", "string value = 3.14;", 0);
    testCase("Invalid float (multiple decimals)", "float bad = 3.14.15;", 0);
    testCase("Invalid float (no digits)", "float empty = .;", 0);
    testCase("Invalid float to int variable", "int bad = 3.14;", 0);
    testCase("Missing expected '\"' on input string (at the start)", "string input = \"hello;", 0);
    testCase("Missing expected '\"' on input string (at the end)", "string input = hello\";", 0);
}

void testCommentCases(void) {
    testTokenCount("void code comment", "// hello world", 0);
    testTokenCount("void code comment with newline", "// hello world \n", 0);
    testTokenCount("Multiple comment lines", "// line 1\n// line 2", 0);
    testTokenCount("Comment with spaces", "   // spaced comment   ", 0);
    testTokenCount("Empty input", "", 0);
    testTokenCount("Only whitespace", "   \n  \t  ", 0);
}

void runTests(void) {
    printf("Starting compiler tests...\n\n");

    testsRun = 0;
    testsPassed = 0;

    testBasicCases();
    testErrorCases();
    testCommentCases();

    printf("=== TEST SUMMARY ===\n");
    printf("Tests run: %d\n", testsRun);
    printf("Tests passed: %d\n", testsPassed);
    printf("Tests failed: %d\n", testsRun - testsPassed);

    if (testsPassed == testsRun) {
        printf("🎉 All tests passed!\n");
    } else {
        printf("⚠️ Some tests failed.\n");
    }

    printf("Success rate: %.1f%%\n",
           testsRun > 0 ? (float) testsPassed / testsRun * 100.0 : 0.0);
}
