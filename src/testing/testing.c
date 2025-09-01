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
    testCase("redefine variable", "sum = 5;", 1);
    testTokenCount("Basic int declaration", "int x = 5;", 5);
    testCase("Basic string declaration", "string name = \"Pablo\";", 1);
    testCase("Basic float declaration", "float pi = 3.14;", 1);
    testCase("Simple addition", "int sum = a + b;", 1);
    testCase("Simple sub", "int sum = a - b;", 1);
    testTokenCount("Simple sub input together", "int sum = a-b;", 7);
    testTokenCount("Simple addition", "int sum = a + b;", 7);
    testCase("Simple subtraction", "int diff = x - y;", 1);
    testCase("Empty string", "string empty = \"\";", 1);
    testCase("String with spaces", "string text = \"hello world\";", 1);
    testCase("String with numbers", "string mixed = \"abc123\";", 1);
    testCase("Multiple assignments", "int a = 1; int b = 2; int c = 3;", 1);
    testCase("Variable reuse", "int x = a; int y = x;", 1);
    //edging
    testCase("Zero assignment", "int zero = 0;", 1);
    testCase("Float without leading zero", "float small = .5;", 1);
    // Mixed declarations and assignments
    testCase("Declaration then assignment", "int x = 5; x = 10;", 1);
    testCase("Multiple assignments", "x = 1; y = 2; z = 3;", 1);
    testCase("Assignment with expression", "result = a + b;", 1);
    testCase("Compound assignment with expression", "total += x + y;", 1);

    //negative numbers
    testCase("Basic negative int", "int x = -5;", 1);
    testCase("Basic negative float", "float pi = -3.14;", 1);
    testCase("Negative at start", "int first = -42;", 1);
    testCase("Negative in addition", "int result = -5 + 3;", 1);
    testCase("Multiple negatives", "int a = -1; int b = -2;", 1);
    // Error cases - should FAIL
    testCase("Negative int to string", "string bad = -123;", 0);
    testCase("Negative float to int", "int bad = -3.14;", 0);
    testCase("Invalid negative float", "float bad = -3.14.15;", 0);
    testCase("Just minus sign", "int bad = -;", 0);
    testCase("Minus with space", "int bad = - 5;", 0);
    // Negative numbers (single tokens)
    testTokenCount("Single negative int", "-42", 1);
    testTokenCount("Assignment with negative", "x = -5", 3);
    testTokenCount("Full statement", "int num = -123", 4);
    //compoundOperations
    testCase("Precedence: 2 + 3 * 4 (should be 2+(3*4))", "int result = 2 + 3 * 4;", 1);
    testCase("Multiple mults: 2 * 2 + 4 * 3 (should be (2*2)+(4*3))", "int result = 2 * 2 + 4 * 3;", 1);
    testCase("Left assoc: 10 - 3 - 2 (should be (10-3)-2)", "int result = 10 - 3 - 2;", 1);
    testCase("Div assoc: 12 / 3 / 2 (should be (12/3)/2)", "int result = 12 / 3 / 2;", 1);
    testCase("Complex: 1 + 2 * 3 - 4 / 2 (should be 1+(2*3)-(4/2))", "int result = 1 + 2 * 3 - 4 / 2;", 1);
    // Subtraction (separate tokens)
    testTokenCount("Simple subtraction", "a - b", 3);
    testTokenCount("Assignment subtraction", "result = a - 5", 5);

    // Mixed cases
    testTokenCount("Negative in addition", "sum = -5 + 10", 5);
    testTokenCount("Negative after operator", "a + -5", 3);
    testTokenCount("Double minus", "a - -5", 3);
    testTokenCount("After equals", "= -42", 2);
    testTokenCount("Complex expression", "calc = -10 - 5", 5);
}

void testErrorCases(void) {
    printf("=== TESTING ERROR CASES ===\n");

    testCase("String to int error", "int x = \"hello\";", 0);
    testCase("Int to string error", "string text = 123;", 0);
    testCase("Float to string error", "string value = 3.14;", 0);
    testCase("Invalid float (multiple decimals)", "float bad = 3.14.15;", 0);
    testCase("Invalid float (no digits)", "float empty = .;", 0);
    testCase("Invalid float to int variable", "int bad = 3.14;", 0);
    // FIX: GIVES ALSO UNKNOWN ERROR ↓↓↓
    testCase("Missing expected '\"' on input string (at the start)", "string input = \"hello;", 0);
    testCase("Missing expected '\"' on input string (at the end)", "string input = hello\";", 0);
    testCase("Invalid Expresion", "int num = -;", 0);
}

void testCommentCases(void) {
    testTokenCount("void code comment", "// hello world", 0);
    testTokenCount("void code comment with newline", "// hello world \n", 0);
    testTokenCount("Multiple comment lines", "// line 1\n// line 2", 0);
    testTokenCount("Comment with spaces", "   // spaced comment   ", 0);
    testTokenCount("Empty input", "", 0);
    testTokenCount("Only whitespace", "   \n  \t  ", 0);
    testTokenCount("Code with inline comment", "int x = 5; // comment", 5);
    testTokenCount("Comment then code", "// comment\nint y = 10;", 5);
}

void runTests(void) {
    printf("Starting compiler tests...\n\n");

    testsRun = 0;
    testsPassed = 0;

    testBasicCases();
    testCommentCases();
    testErrorCases();

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
