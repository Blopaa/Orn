// test/test_main.c
#include "unity.h"
#include "lexer.h"
#include "parser.h"
#include "errorHandling.h"

void setUp(void) {
    resetErrorCount();
}

void tearDown(void) {
}

// ========== BASIC DECLARATIONS TESTS ==========

void test_basic_int_declaration(void) {
    Input res = splitter("int x = 5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_string_declaration(void) {
    Input res = splitter("string name = \"Pablo\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_float_declaration(void) {
    Input res = splitter("float pi = 3.14;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_bool_true_declaration(void) {
    Input res = splitter("bool flag = true;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_bool_false_declaration(void) {
    Input res = splitter("bool active = false;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_variable_reassignment(void) {
    Input res = splitter("sum = 5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

// ========== TOKEN COUNT TESTS ==========

void test_basic_int_token_count(void) {
    Input res = splitter("int x = 5;");
    TEST_ASSERT_EQUAL_INT(5, res->n);
    freeInput(res);
}

void test_simple_addition_token_count(void) {
    Input res = splitter("int sum = a + b;");
    TEST_ASSERT_EQUAL_INT(7, res->n);
    freeInput(res);
}

void test_subtraction_together_token_count(void) {
    Input res = splitter("int sum = a-b;");
    TEST_ASSERT_EQUAL_INT(7, res->n);
    freeInput(res);
}

void test_bool_true_token_count(void) {
    Input res = splitter("bool flag = true;");
    TEST_ASSERT_EQUAL_INT(5, res->n);
    freeInput(res);
}

void test_bool_false_token_count(void) {
    Input res = splitter("bool active = false;");
    TEST_ASSERT_EQUAL_INT(5, res->n);
    freeInput(res);
}

// ========== ARITHMETIC OPERATIONS TESTS ==========

void test_simple_addition(void) {
    Input res = splitter("int sum = a + b;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_simple_subtraction(void) {
    Input res = splitter("int diff = x - y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_simple_multiplication(void) {
    Input res = splitter("int product = x * y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_simple_division(void) {
    Input res = splitter("int quotient = x / y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_modulo_operation(void) {
    Input res = splitter("int result = 10 % 3;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

// ========== PRECEDENCE TESTS ==========

void test_precedence_multiply_first(void) {
    Input res = splitter("int result = 2 + 3 * 4;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_multiple_multiplications(void) {
    Input res = splitter("int result = 2 * 2 + 4 * 3;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_left_associativity_subtraction(void) {
    Input res = splitter("int result = 10 - 3 - 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_division_associativity(void) {
    Input res = splitter("int result = 12 / 3 / 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_complex_precedence(void) {
    Input res = splitter("int result = 1 + 2 * 3 - 4 / 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

// ========== NEGATIVE NUMBERS TESTS ==========

void test_basic_negative_int(void) {
    Input res = splitter("int x = -5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_negative_float(void) {
    Input res = splitter("float pi = -3.14;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_negative_in_expression(void) {
    Input res = splitter("int result = -5 + 3;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_single_negative_token(void) {
    Input res = splitter("-42");
    TEST_ASSERT_EQUAL_INT(1, res->n);
    freeInput(res);
}

void test_assignment_with_negative(void) {
    Input res = splitter("x = -5");
    TEST_ASSERT_EQUAL_INT(3, res->n);
    freeInput(res);
}

void test_negative_vs_subtraction(void) {
    Input res = splitter("a - b");
    TEST_ASSERT_EQUAL_INT(3, res->n);
    freeInput(res);
}

// ========== INCREMENT/DECREMENT TESTS ==========

void test_prefix_increment(void) {
    Input res = splitter("int total = ++index;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_prefix_decrement(void) {
    Input res = splitter("int result = --counter;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_postfix_increment(void) {
    Input res = splitter("int old_val = num++;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_postfix_decrement(void) {
    Input res = splitter("int old_val = num--;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_prefix_increment_tokens(void) {
    Input res = splitter("int y = ++b;");
    TEST_ASSERT_EQUAL_INT(6, res->n);
    freeInput(res);
}

void test_mixed_increment_tokens(void) {
    Input res = splitter("int sum = ++a + b++;");
    TEST_ASSERT_EQUAL_INT(9, res->n);
    freeInput(res);
}

// ========== COMPOUND ASSIGNMENTS TESTS ==========

void test_compound_add_assign(void) {
    Input res = splitter("total += x + y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_compound_sub_assign(void) {
    Input res = splitter("counter -= 1;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_compound_mult_assign(void) {
    Input res = splitter("value *= 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_compound_div_assign(void) {
    Input res = splitter("result /= factor;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

// ========== SPECIAL VALUES TESTS ==========

void test_zero_assignment(void) {
    Input res = splitter("int zero = 0;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_float_without_leading_zero(void) {
    Input res = splitter("float small = .5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_empty_string(void) {
    Input res = splitter("string empty = \"\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_string_with_spaces(void) {
    Input res = splitter("string text = \"hello world\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

// ========== COMMENT TESTS ==========

void test_void_comment(void) {
    Input res = splitter("// hello world");
    TEST_ASSERT_EQUAL_INT(0, res->n);
    freeInput(res);
}

void test_comment_with_newline(void) {
    Input res = splitter("// hello world \n");
    TEST_ASSERT_EQUAL_INT(0, res->n);
    freeInput(res);
}

void test_multiple_comment_lines(void) {
    Input res = splitter("// line 1\n// line 2");
    TEST_ASSERT_EQUAL_INT(0, res->n);
    freeInput(res);
}

void test_empty_input(void) {
    Input res = splitter("");
    TEST_ASSERT_EQUAL_INT(0, res->n);
    freeInput(res);
}

void test_only_whitespace(void) {
    Input res = splitter("   \n  \t  ");
    TEST_ASSERT_EQUAL_INT(0, res->n);
    freeInput(res);
}

void test_code_with_inline_comment(void) {
    Input res = splitter("int x = 5; // comment");
    TEST_ASSERT_EQUAL_INT(5, res->n);
    freeInput(res);
}

void test_comment_then_code(void) {
    Input res = splitter("// comment\nint y = 10;");
    TEST_ASSERT_EQUAL_INT(5, res->n);
    freeInput(res);
}

// ========== MAIN TEST RUNNER ==========

int main(void) {
    UNITY_BEGIN();

    printf("\n=== BASIC DECLARATIONS TESTS ===\n");
    RUN_TEST(test_basic_int_declaration);
    RUN_TEST(test_basic_string_declaration);
    RUN_TEST(test_basic_float_declaration);
    RUN_TEST(test_basic_bool_true_declaration);
    RUN_TEST(test_basic_bool_false_declaration);
    RUN_TEST(test_variable_reassignment);

    printf("\n=== TOKEN COUNT TESTS ===\n");
    RUN_TEST(test_basic_int_token_count);
    RUN_TEST(test_simple_addition_token_count);
    RUN_TEST(test_subtraction_together_token_count);
    RUN_TEST(test_bool_true_token_count);
    RUN_TEST(test_bool_false_token_count);

    printf("\n=== ARITHMETIC OPERATIONS TESTS ===\n");
    RUN_TEST(test_simple_addition);
    RUN_TEST(test_simple_subtraction);
    RUN_TEST(test_simple_multiplication);
    RUN_TEST(test_simple_division);
    RUN_TEST(test_modulo_operation);

    printf("\n=== PRECEDENCE TESTS ===\n");
    RUN_TEST(test_precedence_multiply_first);
    RUN_TEST(test_multiple_multiplications);
    RUN_TEST(test_left_associativity_subtraction);
    RUN_TEST(test_division_associativity);
    RUN_TEST(test_complex_precedence);

    printf("\n=== NEGATIVE NUMBERS TESTS ===\n");
    RUN_TEST(test_basic_negative_int);
    RUN_TEST(test_basic_negative_float);
    RUN_TEST(test_negative_in_expression);
    RUN_TEST(test_single_negative_token);
    RUN_TEST(test_assignment_with_negative);
    RUN_TEST(test_negative_vs_subtraction);

    printf("\n=== INCREMENT/DECREMENT TESTS ===\n");
    RUN_TEST(test_prefix_increment);
    RUN_TEST(test_prefix_decrement);
    RUN_TEST(test_postfix_increment);
    RUN_TEST(test_postfix_decrement);
    RUN_TEST(test_prefix_increment_tokens);
    RUN_TEST(test_mixed_increment_tokens);

    printf("\n=== COMPOUND ASSIGNMENTS TESTS ===\n");
    RUN_TEST(test_compound_add_assign);
    RUN_TEST(test_compound_sub_assign);
    RUN_TEST(test_compound_mult_assign);
    RUN_TEST(test_compound_div_assign);

    printf("\n=== SPECIAL VALUES TESTS ===\n");
    RUN_TEST(test_zero_assignment);
    RUN_TEST(test_float_without_leading_zero);
    RUN_TEST(test_empty_string);
    RUN_TEST(test_string_with_spaces);

    printf("\n=== COMMENT TESTS ===\n");
    RUN_TEST(test_void_comment);
    RUN_TEST(test_comment_with_newline);
    RUN_TEST(test_multiple_comment_lines);
    RUN_TEST(test_empty_input);
    RUN_TEST(test_only_whitespace);
    RUN_TEST(test_code_with_inline_comment);
    RUN_TEST(test_comment_then_code);

    return UNITY_END();
}