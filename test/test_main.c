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

    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_string_declaration(void) {
    Input res = splitter("string name = \"Pablo\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_float_declaration(void) {
    Input res = splitter("float pi = 3.14;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_bool_true_declaration(void) {
    Input res = splitter("bool flag = true;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_bool_false_declaration(void) {
    Input res = splitter("bool active = false;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_variable_reassignment(void) {
    Input res = splitter("sum = 5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

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

    freeTokenList(tokens);
    freeAST(ast);
}

void test_simple_subtraction(void) {
    Input res = splitter("int diff = x - y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_simple_multiplication(void) {
    Input res = splitter("int product = x * y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_simple_division(void) {
    Input res = splitter("int quotient = x / y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_modulo_operation(void) {
    Input res = splitter("int result = 10 % 3;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

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

    freeTokenList(tokens);
    freeAST(ast);
}

void test_multiple_multiplications(void) {
    Input res = splitter("int result = 2 * 2 + 4 * 3;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_left_associativity_subtraction(void) {
    Input res = splitter("int result = 10 - 3 - 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_division_associativity(void) {
    Input res = splitter("int result = 12 / 3 / 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_complex_precedence(void) {
    Input res = splitter("int result = 1 + 2 * 3 - 4 / 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

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

    freeTokenList(tokens);
    freeAST(ast);
}

void test_basic_negative_float(void) {
    Input res = splitter("float pi = -3.14;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_negative_in_expression(void) {
    Input res = splitter("int result = -5 + 3;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_single_negative_token(void) {
    Input res = splitter("-42");
    TEST_ASSERT_EQUAL_INT(2, res->n);
    freeInput(res);
}

void test_assignment_with_negative(void) {
    Input res = splitter("x = -5");
    TEST_ASSERT_EQUAL_INT(4, res->n);
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

    freeTokenList(tokens);
    freeAST(ast);
}

void test_prefix_decrement(void) {
    Input res = splitter("int result = --counter;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_postfix_increment(void) {
    Input res = splitter("int old_val = num++;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_postfix_decrement(void) {
    Input res = splitter("int old_val = num--;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

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

    freeTokenList(tokens);
    freeAST(ast);
}

void test_compound_sub_assign(void) {
    Input res = splitter("counter -= 1;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_compound_mult_assign(void) {
    Input res = splitter("value *= 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_compound_div_assign(void) {
    Input res = splitter("result /= factor;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

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

    freeTokenList(tokens);
    freeAST(ast);
}

void test_float_without_leading_zero(void) {
    Input res = splitter("float small = .5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_empty_string(void) {
    Input res = splitter("string empty = \"\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_string_with_spaces(void) {
    Input res = splitter("string text = \"hello world\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

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
    Input res = splitter("    \n  \t  ");
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

// ========== LOGICAL OPERATORS TESTS (MISSING) ==========

void test_logical_and_operation(void) {
    Input res = splitter("bool result = true && false;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_logical_or_operation(void) {
    Input res = splitter("bool result = x > 0 || y < 10;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_logical_not_operation(void) {
    Input res = splitter("bool result = !active;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_complex_logical_expression(void) {
    Input res = splitter("bool result = x > 0 && y < 10 || !flag;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_logical_precedence(void) {
    Input res = splitter("bool result = a && b || c && d;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== COMPARISON OPERATORS TESTS (MISSING) ==========

void test_equal_comparison(void) {
    Input res = splitter("bool equal = x == y;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_not_equal_comparison(void) {
    Input res = splitter("bool not_equal = a != b;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_less_than_comparison(void) {
    Input res = splitter("bool less = value < limit;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_greater_than_comparison(void) {
    Input res = splitter("bool greater = score > 90;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_less_equal_comparison(void) {
    Input res = splitter("bool less_eq = age <= 65;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_greater_equal_comparison(void) {
    Input res = splitter("bool greater_eq = temp >= 0;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_chained_comparisons(void) {
    Input res = splitter("bool range_check = min <= value && value <= max;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_comparison_with_arithmetic(void) {
    Input res = splitter("bool result = a + b >= c * 2;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== MIXED OPERATOR PRECEDENCE TESTS (MISSING) ==========

void test_logical_and_precedence_over_or(void) {
    Input res = splitter("bool result = a || b && c;"); // Should be a || (b && c)
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_comparison_precedence_over_logical(void) {
    Input res = splitter("bool result = a > b && c < d;"); // Should be (a > b) && (c < d)
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_arithmetic_precedence_over_comparison(void) {
    Input res = splitter("bool result = a + b < c * d;"); // Should be (a + b) < (c * d)
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_unary_precedence_over_arithmetic(void) {
    Input res = splitter("int result = !x + y;"); // Should be (!x) + y
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== FLOAT VALIDATION TESTS (MORE EDGE CASES) ==========

void test_float_with_leading_zero(void) {
    Input res = splitter("float value = 0.123;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_float_integer_part_only(void) {
    Input res = splitter("float value = 123.0;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_negative_float_without_leading_zero(void) {
   Input res = splitter("float value = -.5;");
   Token tokens = tokenization(res);
   ASTNode ast = ASTGenerator(tokens);

   TEST_ASSERT_NOT_NULL(ast);
   TEST_ASSERT_FALSE(hasErrors());

   freeTokenList(tokens);
   freeAST(ast);
}

// ========== ERROR HANDLING TESTS (MISSING) ==========

void test_invalid_float_multiple_decimals(void) {
    resetErrorCount();
    Input res = splitter("float bad = 3.14.15;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_TRUE(hasErrors());
    TEST_ASSERT_EQUAL_INT(1, getErrorCount());

    freeTokenList(tokens);
    if (ast) freeAST(ast);
}

void test_invalid_float_no_digits(void) {
    resetErrorCount();
    Input res = splitter("float bad = .;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_TRUE(hasErrors());

    freeTokenList(tokens);
    if (ast) freeAST(ast);
}

void test_invalid_float_invalid_char(void) {
    resetErrorCount();
    Input res = splitter("float bad = 3.1a4;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_TRUE(hasErrors());

    freeTokenList(tokens);
    if (ast) freeAST(ast);
}

void test_invalid_string_missing_quote(void) {
    resetErrorCount();
    Input res = splitter("string bad = \"hello;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_TRUE(hasErrors());

    freeTokenList(tokens);
    if (ast) freeAST(ast);
}

void test_invalid_expression(void) {
    resetErrorCount();
    Input res = splitter("int bad = + * 5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_TRUE(hasErrors());

    freeTokenList(tokens);
    if (ast) freeAST(ast);
}

// ========== COMPLEX EXPRESSION TESTS (MISSING) ==========

void test_complex_mixed_operations(void) {
    Input res = splitter("bool result = a++ + --b > c * 2 && !flag;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_multiple_logical_operators(void) {
    Input res = splitter("bool check = a > 0 && b < 10 || c == 5 && !d;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_nested_increment_operations(void) {
    Input res = splitter("int result = ++a + b++ - --c + d--;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_complex_compound_assignment(void) {
    Input res = splitter("total += x++ * --y + z;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== EDGE CASE TESTS (MISSING) ==========

void test_multiple_statements(void) {
    Input res = splitter("int x = 5; float y = 3.14; bool flag = true;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_long_variable_names(void) {
    Input res = splitter("int very_long_variable_name_with_underscores = 42;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_underscore_variable_names(void) {
    Input res = splitter("int _private_var = 10;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_numbers_with_multiple_digits(void) {
    Input res = splitter("int big = 123456789;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_very_long_string(void) {
    Input res = splitter("string text = \"This is a very long string with many characters to test string handling\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== TOKEN COUNTING EDGE CASES (MISSING) ==========

void test_logical_operators_token_count(void) {
    Input res = splitter("bool result = a && b || !c;");
    TEST_ASSERT_EQUAL_INT(10, res->n); // bool result = a && b || ! c ;
    freeInput(res);
}

void test_comparison_operators_token_count(void) {
    Input res = splitter("bool check = x >= y && z <= w;");
    TEST_ASSERT_EQUAL_INT(11, res->n); // bool check = x >= y && z <= w ;
    freeInput(res);
}

void test_complex_expression_token_count(void) {
    Input res = splitter("bool result = a++ + b-- >= c * 2;");
    TEST_ASSERT_EQUAL_INT(13, res->n);
    freeInput(res);
}

void test_multiple_compound_assignments_token_count(void) {
    Input res = splitter("x += 5; y -= 3; z *= 2;");
    TEST_ASSERT_EQUAL_INT(12, res->n); // x += 5 ; y -= 3 ; z *= 2 ;
    freeInput(res);
}

// ========== WHITESPACE AND FORMATTING TESTS (MISSING) ==========

void test_extra_whitespace_handling(void) {
    Input res = splitter("    int     x   =    5   ;    ");
    TEST_ASSERT_EQUAL_INT(5, res->n);
    freeInput(res);
}

void test_mixed_whitespace_and_tabs(void) {
    Input res = splitter("\tint\tx\t=\t5;\t");
    TEST_ASSERT_EQUAL_INT(5, res->n);
    freeInput(res);
}

void test_no_spaces_between_operators(void) {
    Input res = splitter("int result=a+b*c;");
    TEST_ASSERT_EQUAL_INT(9, res->n);
    freeInput(res);
}

void test_newlines_in_code(void) {
    Input res = splitter("int x = 5;\nfloat y = 3.14;\n");
    TEST_ASSERT_EQUAL_INT(10, res->n);
    freeInput(res);
}

// ========== BLOCK STATEMENT TESTS ==========

void test_empty_block(void) {
    Input res = splitter("{}");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    // Should have one child (the block)
    TEST_ASSERT_NOT_NULL(ast->children);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, ast->children->NodeType);

    // Block should have no children (empty)
    TEST_ASSERT_NULL(ast->children->children);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_block_with_single_statement(void) {
    Input res = splitter("{ int x = 5; }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    // Should have one child (the block)
    TEST_ASSERT_NOT_NULL(ast->children);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, ast->children->NodeType);

    // Block should have one child (the statement)
    TEST_ASSERT_NOT_NULL(ast->children->children);
    TEST_ASSERT_EQUAL_INT(INT_VARIABLE_DEFINITION, ast->children->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_block_with_multiple_statements(void) {
    Input res = splitter("{ int x = 5; string name = \"test\"; bool flag = true; }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode block = ast->children;
    TEST_ASSERT_NOT_NULL(block);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, block->NodeType);

    // Check first statement
    ASTNode stmt1 = block->children;
    TEST_ASSERT_NOT_NULL(stmt1);
    TEST_ASSERT_EQUAL_INT(INT_VARIABLE_DEFINITION, stmt1->NodeType);

    // Check second statement
    ASTNode stmt2 = stmt1->brothers;
    TEST_ASSERT_NOT_NULL(stmt2);
    TEST_ASSERT_EQUAL_INT(STRING_VARIABLE_DEFINITION, stmt2->NodeType);

    // Check third statement
    ASTNode stmt3 = stmt2->brothers;
    TEST_ASSERT_NOT_NULL(stmt3);
    TEST_ASSERT_EQUAL_INT(BOOL_VARIABLE_DEFINITION, stmt3->NodeType);

    // Should be no more statements
    TEST_ASSERT_NULL(stmt3->brothers);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_nested_blocks(void) {
    Input res = splitter("{ int x = 5; { string name = \"nested\"; } }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode outerBlock = ast->children;
    TEST_ASSERT_NOT_NULL(outerBlock);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, outerBlock->NodeType);

    // First child should be the int declaration
    ASTNode stmt1 = outerBlock->children;
    TEST_ASSERT_NOT_NULL(stmt1);
    TEST_ASSERT_EQUAL_INT(INT_VARIABLE_DEFINITION, stmt1->NodeType);

    // Second child should be the inner block
    ASTNode innerBlock = stmt1->brothers;
    TEST_ASSERT_NOT_NULL(innerBlock);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, innerBlock->NodeType);

    // Inner block should have one statement
    ASTNode innerStmt = innerBlock->children;
    TEST_ASSERT_NOT_NULL(innerStmt);
    TEST_ASSERT_EQUAL_INT(STRING_VARIABLE_DEFINITION, innerStmt->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_block_with_assignments(void) {
    Input res = splitter("{ x = 10; y += 5; z *= 2; }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode block = ast->children;
    TEST_ASSERT_NOT_NULL(block);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, block->NodeType);

    // Check assignment
    ASTNode assign = block->children;
    TEST_ASSERT_NOT_NULL(assign);
    TEST_ASSERT_EQUAL_INT(ASSIGNMENT, assign->NodeType);

    // Check compound assignments
    ASTNode compoundAdd = assign->brothers;
    TEST_ASSERT_NOT_NULL(compoundAdd);
    TEST_ASSERT_EQUAL_INT(COMPOUND_ADD_ASSIGN, compoundAdd->NodeType);

    ASTNode compoundMul = compoundAdd->brothers;
    TEST_ASSERT_NOT_NULL(compoundMul);
    TEST_ASSERT_EQUAL_INT(COMPOUND_MUL_ASSIGN, compoundMul->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_block_with_expressions(void) {
    Input res = splitter("{ int result = a + b * c; bool check = x >= y; }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode block = ast->children;
    TEST_ASSERT_NOT_NULL(block);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, block->NodeType);

    // Check first statement with arithmetic expression
    ASTNode stmt1 = block->children;
    TEST_ASSERT_NOT_NULL(stmt1);
    TEST_ASSERT_EQUAL_INT(INT_VARIABLE_DEFINITION, stmt1->NodeType);

    // Check second statement with comparison expression
    ASTNode stmt2 = stmt1->brothers;
    TEST_ASSERT_NOT_NULL(stmt2);
    TEST_ASSERT_EQUAL_INT(BOOL_VARIABLE_DEFINITION, stmt2->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_multiple_top_level_blocks(void) {
    Input res = splitter("{ int x = 1; } { string y = \"test\"; }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    // Should have two top-level blocks
    ASTNode block1 = ast->children;
    TEST_ASSERT_NOT_NULL(block1);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, block1->NodeType);

    ASTNode block2 = block1->brothers;
    TEST_ASSERT_NOT_NULL(block2);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, block2->NodeType);

    // First block should have int declaration
    TEST_ASSERT_NOT_NULL(block1->children);
    TEST_ASSERT_EQUAL_INT(INT_VARIABLE_DEFINITION, block1->children->NodeType);

    // Second block should have string declaration
    TEST_ASSERT_NOT_NULL(block2->children);
    TEST_ASSERT_EQUAL_INT(STRING_VARIABLE_DEFINITION, block2->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== BLOCK TOKEN COUNT TESTS ==========

void test_empty_block_token_count(void) {
    Input res = splitter("{}");
    TEST_ASSERT_EQUAL_INT(2, res->n); // { and }
    freeInput(res);
}

void test_block_with_statement_token_count(void) {
    Input res = splitter("{ int x = 5; }");
    TEST_ASSERT_EQUAL_INT(7, res->n); // {, int, x, =, 5, ;, }
    freeInput(res);
}

void test_nested_block_token_count(void) {
    Input res = splitter("{ { } }");
    TEST_ASSERT_EQUAL_INT(4, res->n); // {, {, }, }
    freeInput(res);
}

// ========== COMPLEX BLOCK SCENARIOS ==========

void test_deeply_nested_blocks(void) {
    Input res = splitter("{ int a = 1; { float b = 2.5; { bool c = true; } } }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    // Navigate through the nested structure
    ASTNode level1 = ast->children;
    TEST_ASSERT_NOT_NULL(level1);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, level1->NodeType);

    ASTNode stmt1 = level1->children;
    TEST_ASSERT_NOT_NULL(stmt1);
    TEST_ASSERT_EQUAL_INT(INT_VARIABLE_DEFINITION, stmt1->NodeType);

    ASTNode level2 = stmt1->brothers;
    TEST_ASSERT_NOT_NULL(level2);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, level2->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_block_with_mixed_statements(void) {
    Input res = splitter("{ int x = 5; y = x + 10; z += y; bool flag = x > 0; }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode block = ast->children;
    TEST_ASSERT_NOT_NULL(block);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, block->NodeType);

    // Should have 4 statements in the block
    ASTNode stmt = block->children;
    int stmtCount = 0;
    while (stmt != NULL) {
        stmtCount++;
        stmt = stmt->brothers;
    }
    TEST_ASSERT_EQUAL_INT(4, stmtCount);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_empty_nested_blocks(void) {
    Input res = splitter("{ {} {} }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode outerBlock = ast->children;
    TEST_ASSERT_NOT_NULL(outerBlock);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, outerBlock->NodeType);

    // Should have two empty inner blocks
    ASTNode innerBlock1 = outerBlock->children;
    TEST_ASSERT_NOT_NULL(innerBlock1);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, innerBlock1->NodeType);
    TEST_ASSERT_NULL(innerBlock1->children); // Should be empty

    ASTNode innerBlock2 = innerBlock1->brothers;
    TEST_ASSERT_NOT_NULL(innerBlock2);
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, innerBlock2->NodeType);
    TEST_ASSERT_NULL(innerBlock2->children); // Should be empty

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== BASIC IF-CONDITIONAL TESTS ==========

void test_classic_ternary_assignment(void) {
    Input res = splitter("int result = x > 0 ? y + 1 : y - 1;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode varDef = ast->children;
    ASTNode ifCond = varDef->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, ifCond->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_if_only_with_assignment(void) {
    Input res = splitter("authenticated ? x = 1;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, ifCond->NodeType);

    // Should have true branch only
    ASTNode trueBranch = ifCond->children->brothers;
    TEST_ASSERT_EQUAL_INT(IF_TRUE_BRANCH, trueBranch->NodeType);
    TEST_ASSERT_NULL(trueBranch->brothers);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_if_only_with_block(void) {
    Input res = splitter("condition ? { x = 5; y++; };");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, ifCond->NodeType);

    ASTNode trueBranch = ifCond->children->brothers;
    TEST_ASSERT_EQUAL_INT(IF_TRUE_BRANCH, trueBranch->NodeType);
    TEST_ASSERT_EQUAL_INT(BLOCK_EXPRESSION, trueBranch->children->NodeType);
    TEST_ASSERT_NULL(trueBranch->brothers);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_enhanced_ternary_with_blocks(void) {
    Input res = splitter("x > 0 ? { result = x * 2; } : { result = 0; };");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, ifCond->NodeType);

    ASTNode trueBranch = ifCond->children->brothers;
    TEST_ASSERT_EQUAL_INT(IF_TRUE_BRANCH, trueBranch->NodeType);
    TEST_ASSERT_EQUAL_INT(BLOCK_EXPRESSION, trueBranch->children->NodeType);

    ASTNode falseBranch = trueBranch->brothers;
    TEST_ASSERT_EQUAL_INT(ELSE_BRANCH, falseBranch->NodeType);
    TEST_ASSERT_EQUAL_INT(BLOCK_EXPRESSION, falseBranch->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== MIXED EXPRESSION TESTS ==========

void test_mixed_block_and_expression(void) {
    Input res = splitter("error ? { x = 0; y = -1; } : default_value;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    ASTNode trueBranch = ifCond->children->brothers;
    ASTNode falseBranch = trueBranch->brothers;

    TEST_ASSERT_EQUAL_INT(BLOCK_EXPRESSION, trueBranch->children->NodeType);
    TEST_ASSERT_NOT_EQUAL_INT(BLOCK_EXPRESSION, falseBranch->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_mixed_expression_and_block(void) {
    Input res = splitter("simple ? value : { result = x + y; count++; };");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    ASTNode trueBranch = ifCond->children->brothers;
    ASTNode falseBranch = trueBranch->brothers;

    TEST_ASSERT_NOT_EQUAL_INT(BLOCK_EXPRESSION, trueBranch->children->NodeType);
    TEST_ASSERT_EQUAL_INT(BLOCK_EXPRESSION, falseBranch->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== PRECEDENCE TESTS ==========

void test_right_associativity(void) {
    Input res = splitter("a ? b : c ? d : e;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    // Should parse as: a ? b : (c ? d : e)
    ASTNode outerIf = ast->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, outerIf->NodeType);

    ASTNode falseBranch = outerIf->children->brothers->brothers;
    ASTNode innerIf = falseBranch->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, innerIf->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_precedence_with_arithmetic(void) {
    Input res = splitter("int result = a + b > 0 ? x * 2 : y + 1;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode varDef = ast->children;
    ASTNode ifCond = varDef->children;
    ASTNode condition = ifCond->children;

    TEST_ASSERT_EQUAL_INT(GREATER_THAN_OP, condition->NodeType);
    TEST_ASSERT_EQUAL_INT(ADD_OP, condition->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_precedence_with_logical(void) {
    Input res = splitter("bool result = x > 0 && y < 10 ? true : false;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode varDef = ast->children;
    ASTNode ifCond = varDef->children;
    ASTNode condition = ifCond->children;

    TEST_ASSERT_EQUAL_INT(LOGIC_AND, condition->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== COMPOUND OPERATIONS ==========

void test_with_compound_assignments(void) {
    Input res = splitter("score >= 90 ? grade += 10 : grade -= 5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    ASTNode trueBranch = ifCond->children->brothers;
    ASTNode falseBranch = trueBranch->brothers;

    TEST_ASSERT_EQUAL_INT(COMPOUND_ADD_ASSIGN, trueBranch->children->NodeType);
    TEST_ASSERT_EQUAL_INT(COMPOUND_SUB_ASSIGN, falseBranch->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_with_increment_decrement(void) {
    Input res = splitter("positive ? ++counter : --counter;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    ASTNode trueBranch = ifCond->children->brothers;
    ASTNode falseBranch = trueBranch->brothers;

    TEST_ASSERT_EQUAL_INT(PRE_INCREMENT, trueBranch->children->NodeType);
    TEST_ASSERT_EQUAL_INT(PRE_DECREMENT, falseBranch->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_with_postfix_operations(void) {
    Input res = splitter("flag ? x++ : y--;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    ASTNode trueBranch = ifCond->children->brothers;
    ASTNode falseBranch = trueBranch->brothers;

    TEST_ASSERT_EQUAL_INT(POST_INCREMENT, trueBranch->children->NodeType);
    TEST_ASSERT_EQUAL_INT(POST_DECREMENT, falseBranch->children->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== NESTED TESTS ==========

void test_nested_conditionals(void) {
    Input res = splitter("a ? b ? c : d : e;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode outerIf = ast->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, outerIf->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_nested_in_blocks(void) {
    Input res = splitter("admin ? { permission ? x = 1 : x = 0; } : x = -1;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, ifCond->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_multiple_conditional_statements(void) {
    Input res = splitter("condition1 ? x = 1; condition2 ? y = 2 : z = 3;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode firstIf = ast->children;
    ASTNode secondIf = firstIf->brothers;

    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, firstIf->NodeType);
    TEST_ASSERT_EQUAL_INT(IF_CONDITIONAL, secondIf->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== TYPE-SPECIFIC TESTS ==========

void test_string_conditionals(void) {
    Input res = splitter("string msg = logged_in ? \"Welcome\" : \"Please login\";");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode varDef = ast->children;
    TEST_ASSERT_EQUAL_INT(STRING_VARIABLE_DEFINITION, varDef->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_float_conditionals(void) {
    Input res = splitter("float rate = premium ? 2.5 : 1.0;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode varDef = ast->children;
    TEST_ASSERT_EQUAL_INT(FLOAT_VARIABLE_DEFINITION, varDef->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_bool_conditionals(void) {
    Input res = splitter("bool active = count > 0 ? true : false;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode varDef = ast->children;
    TEST_ASSERT_EQUAL_INT(BOOL_VARIABLE_DEFINITION, varDef->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== EDGE CASES ==========

void test_empty_blocks(void) {
    Input res = splitter("condition ? {} : {};");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

void test_if_only_empty_block(void) {
    Input res = splitter("condition ? {};");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    ASTNode trueBranch = ifCond->children->brothers;

    TEST_ASSERT_EQUAL_INT(IF_TRUE_BRANCH, trueBranch->NodeType);
    TEST_ASSERT_NULL(trueBranch->brothers);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_complex_boolean_condition(void) {
    Input res = splitter("x > 0 && y <= 100 || !flag ? result = 1 : result = 0;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode ifCond = ast->children;
    ASTNode condition = ifCond->children;

    TEST_ASSERT_EQUAL_INT(LOGIC_OR, condition->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

void test_negative_numbers(void) {
    Input res = splitter("x < 0 ? result = -1 : result = -x;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    freeTokenList(tokens);
    freeAST(ast);
}

// loops

void test_basic_while_loop(void) {
    Input res = splitter("@ x > 0 { x--; }");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);

    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());

    ASTNode whileLoop = ast->children;
    TEST_ASSERT_EQUAL_INT(LOOP_STATEMENT, whileLoop->NodeType);

    // Check condition
    ASTNode condition = whileLoop->children;
    TEST_ASSERT_EQUAL_INT(GREATER_THAN_OP, condition->NodeType);

    // Check body
    ASTNode body = condition->brothers;
    TEST_ASSERT_EQUAL_INT(BLOCK_STATEMENT, body->NodeType);

    freeTokenList(tokens);
    freeAST(ast);
}

// ========== TOKEN COUNT TESTS ==========

void test_classic_ternary_tokens(void) {
    Input res = splitter("x > 0 ? y : z");
    TEST_ASSERT_EQUAL_INT(7, res->n);
    freeInput(res);
}

void test_if_only_tokens(void) {
    Input res = splitter("condition ? x++");
    TEST_ASSERT_EQUAL_INT(4, res->n);
    freeInput(res);
}

void test_block_tokens(void) {
    Input res = splitter("x ? { a = 1; } : { b = 2; }");
    TEST_ASSERT_EQUAL_INT(15, res->n);
    freeInput(res);
}

void test_nested_tokens(void) {
    Input res = splitter("a ? b : c ? d : e");
    TEST_ASSERT_EQUAL_INT(9, res->n);
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

    printf("\n=== LOGICAL OPERATORS TESTS ===\n");
    RUN_TEST(test_logical_and_operation);
    RUN_TEST(test_logical_or_operation);
    RUN_TEST(test_logical_not_operation);
    RUN_TEST(test_complex_logical_expression);
    RUN_TEST(test_logical_precedence);

    printf("\n=== COMPARISON OPERATORS TESTS ===\n");
    RUN_TEST(test_equal_comparison);
    RUN_TEST(test_not_equal_comparison);
    RUN_TEST(test_less_than_comparison);
    RUN_TEST(test_greater_than_comparison);
    RUN_TEST(test_less_equal_comparison);
    RUN_TEST(test_greater_equal_comparison);
    RUN_TEST(test_chained_comparisons);
    RUN_TEST(test_comparison_with_arithmetic);

    printf("\n=== MIXED OPERATOR PRECEDENCE TESTS ===\n");
    RUN_TEST(test_logical_and_precedence_over_or);
    RUN_TEST(test_comparison_precedence_over_logical);
    RUN_TEST(test_arithmetic_precedence_over_comparison);
    RUN_TEST(test_unary_precedence_over_arithmetic);

    printf("\n=== FLOAT VALIDATION TESTS ===\n");
    RUN_TEST(test_float_with_leading_zero);
    RUN_TEST(test_float_integer_part_only);
    RUN_TEST(test_negative_float_without_leading_zero);

    printf("\n=== ERROR HANDLING TESTS ===\n");
    RUN_TEST(test_invalid_float_multiple_decimals);
    RUN_TEST(test_invalid_float_no_digits);
    RUN_TEST(test_invalid_float_invalid_char);
    RUN_TEST(test_invalid_string_missing_quote);
    RUN_TEST(test_invalid_expression);

    printf("\n=== COMPLEX EXPRESSION TESTS ===\n");
    RUN_TEST(test_complex_mixed_operations);
    RUN_TEST(test_multiple_logical_operators);
    RUN_TEST(test_nested_increment_operations);
    RUN_TEST(test_complex_compound_assignment);

    printf("\n=== EDGE CASE TESTS ===\n");
    RUN_TEST(test_multiple_statements);
    RUN_TEST(test_long_variable_names);
    RUN_TEST(test_underscore_variable_names);
    RUN_TEST(test_numbers_with_multiple_digits);
    RUN_TEST(test_very_long_string);

    printf("\n=== TOKEN COUNTING EDGE CASES ===\n");
    RUN_TEST(test_logical_operators_token_count);
    RUN_TEST(test_comparison_operators_token_count);
    RUN_TEST(test_complex_expression_token_count);
    RUN_TEST(test_multiple_compound_assignments_token_count);

    printf("\n=== WHITESPACE AND FORMATTING TESTS ===\n");
    RUN_TEST(test_extra_whitespace_handling);
    RUN_TEST(test_mixed_whitespace_and_tabs);
    RUN_TEST(test_no_spaces_between_operators);
    RUN_TEST(test_newlines_in_code);

    printf("\n=== BLOCK STATEMENT TESTS ===\n");
    RUN_TEST(test_empty_block);
    RUN_TEST(test_block_with_single_statement);
    RUN_TEST(test_block_with_multiple_statements);
    RUN_TEST(test_nested_blocks);
    RUN_TEST(test_block_with_assignments);
    RUN_TEST(test_block_with_expressions);
    RUN_TEST(test_multiple_top_level_blocks);

    printf("\n=== BLOCK TOKEN COUNT TESTS ===\n");
    RUN_TEST(test_empty_block_token_count);
    RUN_TEST(test_block_with_statement_token_count);
    RUN_TEST(test_nested_block_token_count);

    printf("\n=== COMPLEX BLOCK SCENARIOS ===\n");
    RUN_TEST(test_deeply_nested_blocks);
    RUN_TEST(test_block_with_mixed_statements);
    RUN_TEST(test_empty_nested_blocks);
    printf("\n=== IF-CONDITIONAL TESTS ===\n");
    RUN_TEST(test_classic_ternary_assignment);
    RUN_TEST(test_if_only_with_assignment);
    RUN_TEST(test_if_only_with_block);
    RUN_TEST(test_enhanced_ternary_with_blocks);

    printf("\n=== MIXED EXPRESSIONS ===\n");
    RUN_TEST(test_mixed_block_and_expression);
    RUN_TEST(test_mixed_expression_and_block);

    printf("\n=== PRECEDENCE ===\n");
    RUN_TEST(test_right_associativity);
    RUN_TEST(test_precedence_with_arithmetic);
    RUN_TEST(test_precedence_with_logical);

    printf("\n=== COMPOUND OPERATIONS ===\n");
    RUN_TEST(test_with_compound_assignments);
    RUN_TEST(test_with_increment_decrement);
    RUN_TEST(test_with_postfix_operations);

    printf("\n=== NESTING ===\n");
    RUN_TEST(test_nested_conditionals);
    RUN_TEST(test_nested_in_blocks);
    RUN_TEST(test_multiple_conditional_statements);

    printf("\n=== TYPES ===\n");
    RUN_TEST(test_string_conditionals);
    RUN_TEST(test_float_conditionals);
    RUN_TEST(test_bool_conditionals);

    printf("\n=== EDGE CASES ===\n");
    RUN_TEST(test_empty_blocks);
    RUN_TEST(test_if_only_empty_block);
    RUN_TEST(test_complex_boolean_condition);
    RUN_TEST(test_negative_numbers);

    printf("\n=== TOKEN COUNTS ===\n");
    RUN_TEST(test_classic_ternary_tokens);
    RUN_TEST(test_if_only_tokens);
    RUN_TEST(test_block_tokens);
    RUN_TEST(test_nested_tokens);

    printf("\n=== LOOPS ===\n");
    RUN_TEST(test_basic_while_loop);

    return UNITY_END();
}
