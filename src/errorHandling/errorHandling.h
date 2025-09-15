/**
 * @file errorHandling.h
 * @brief Error handling system for the C compiler.
 *
 * Provides comprehensive error reporting with categorized error codes,
 * severity levels, and detailed error tracking functionality.
 * Supports warning, error, and fatal error types with context information.
 */

#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define GRAY    "\033[90m"
#include <stddef.h>

/**
 * @brief Error code enumeration categorized by type.
 *
 * Error codes are organized in ranges:
 * - 1000s: Type mismatch errors
 * - 2000s:  Variable/Symbol errors
 * - 3000s: Syntax errors
 * - 4000s: Logic errors
 * - 5000s: System errors
 */
typedef enum {
	ERROR_OK = 0,
	// 1000S type errors
	ERROR_TYPE_MISMATCH_STRING_TO_INT = 1001,
	ERROR_TYPE_MISMATCH_INT_TO_STRING = 1002,
	ERROR_TYPE_MISMATCH_FLOAT_TO_STRING = 1003,
	ERROR_TYPE_MISMATCH_STRING_TO_FLOAT = 1004,
	ERROR_TYPE_MISMATCH_FLOAT_TO_INT = 1005,
	ERROR_TYPE_MISMATCH_STRING_TO_BOOL = 1006,
	ERROR_TYPE_MISMATCH_FLOAT_TO_BOOL = 1007,
	ERROR_TYPE_MISMATCH_INT_TO_BOOL = 1008,
	ERROR_TYPE_MISMATCH_BOOL_TO_STRING = 1009,
	ERROR_TYPE_MISMATCH_BOOL_TO_INT = 1010,
	ERROR_TYPE_MISMATCH_BOOL_TO_FLOAT = 1011,

	//2000 Variable/Symbol errors
	ERROR_UNDEFINED_VARIABLE = 2001,
	ERROR_VARIABLE_REDECLARED = 2002,
	ERROR_VARIABLE_NOT_INITIALIZED = 2003,
	ERROR_INVALID_VARIABLE_NAME = 2004,

	// 3000s syntax errors
	ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS = 3001,
	ERROR_INVALID_FLOAT_INVALID_CHAR = 3002,
	ERROR_INVALID_FLOAT_NO_DIGITS = 3003,
	ERROR_MISSING_QUOTE = 3004,
	ERROR_INVALID_EXPRESSION = 3005,
	ERROR_UNMATCHED_LEFT_BRACE = 3006,
	ERROR_TERNARY_MISSING_TRUE_BRANCH = 3007,
	ERROR_TERNARY_MISSING_FALSE_BRANCH = 3008,
	ERROR_TERNARY_INVALID_CONDITION = 3009,

	// 4000s: Logic/Flow errors
	ERROR_INVALID_ASSIGNMENT_TARGET = 4001,
	ERROR_INVALID_OPERATION_FOR_TYPE = 4002,
	ERROR_INCOMPATIBLE_OPERAND_TYPES = 4003,
	ERROR_INVALID_UNARY_OPERAND = 4004,

	// 5000s: System/Internal errors
	ERROR_MEMORY_ALLOCATION_FAILED = 5001,
	ERROR_SYMBOL_TABLE_CREATION_FAILED = 5002,
	ERROR_CONTEXT_CREATION_FAILED = 5003,
	ERROR_INTERNAL_PARSER_ERROR = 5004,
	ERROR_INTERNAL_CODE_GENERATOR_ERROR = 5005,
} ErrorCode;

/**
 * @brief Error severity levels in ascending order of severity.
 *
 * Determines the handling behavior:
 * - WARNING: Logged but compilation continues
 * - ERROR: Logged, compilation continues but will fail
 * - FATAL: Logged and compilation immediately aborted
 */
typedef enum {
	WARNING,
	ERROR,
	FATAL
} ErrorLevel;

typedef struct ErrorContext {
	const char* file;
	size_t line;
	size_t column;
	const char* source;
	size_t length;
	size_t startColumn;
} ErrorContext;

typedef struct {
	ErrorCode code;
	ErrorLevel level;
	const char* message;
	const char* note;
	const char* help;
	const char* suggestion;
} ErrorInfo;

static const ErrorInfo errorDatabase[] = {
    // Type mismatch errors (1000s)
    {
        ERROR_TYPE_MISMATCH_STRING_TO_INT,
        ERROR,
        "mismatched types",
        "string literals cannot be assigned to int variables",
        "expected `int`, found `string`",
        "try converting the string to an integer or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_INT_TO_STRING,
        ERROR,
        "mismatched types",
        "integer literals cannot be assigned to string variables",
        "expected `string`, found `int`",
        "try wrapping the number in quotes or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_FLOAT_TO_STRING,
        ERROR,
        "mismatched types",
        "float literals cannot be assigned to string variables",
        "expected `string`, found `float`",
        "try wrapping the number in quotes or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_STRING_TO_FLOAT,
        ERROR,
        "mismatched types",
        "string literals cannot be assigned to float variables",
        "expected `float`, found `string`",
        "try converting the string to a float or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_FLOAT_TO_INT,
        ERROR,
        "mismatched types",
        "float literals lose precision when assigned to int variables",
        "expected `int`, found `float`",
        "try truncating the decimal or change the variable to `float`"
    },
    {
        ERROR_TYPE_MISMATCH_STRING_TO_BOOL,
        ERROR,
        "mismatched types",
        "string literals cannot be assigned to bool variables",
        "expected `bool`, found `string`",
        "try using `true`/`false` or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_FLOAT_TO_BOOL,
        ERROR,
        "mismatched types",
        "float literals cannot be assigned to bool variables",
        "expected `bool`, found `float`",
        "try using a comparison like `value != 0.0` or `true`/`false`"
    },
    {
        ERROR_TYPE_MISMATCH_INT_TO_BOOL,
        ERROR,
        "mismatched types",
        "integer values cannot be directly assigned to bool variables",
        "expected `bool`, found `int`",
        "try using a comparison like `value != 0` or `true`/`false`"
    },
    {
        ERROR_TYPE_MISMATCH_BOOL_TO_STRING,
        ERROR,
        "mismatched types",
        "boolean values cannot be assigned to string variables",
        "expected `string`, found `bool`",
        "try using conditional assignment or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_BOOL_TO_INT,
        ERROR,
        "mismatched types",
        "boolean values cannot be directly assigned to int variables",
        "expected `int`, found `bool`",
        "try using conditional assignment or change variable type"
    },
    {
        ERROR_TYPE_MISMATCH_BOOL_TO_FLOAT,
        ERROR,
        "mismatched types",
        "boolean values cannot be assigned to float variables",
        "expected `float`, found `bool`",
        "try using conditional assignment or change the variable type"
    },

    // Variable/Symbol errors (2000s)
    {
        ERROR_UNDEFINED_VARIABLE,
        ERROR,
        "cannot find value in this scope",
        "this variable has not been declared",
        "variable not found",
        "check the variable name or declare it before use"
    },
    {
        ERROR_VARIABLE_REDECLARED,
        ERROR,
        "variable redeclared",
        "a variable with this name already exists in the current scope",
        "identifier already declared",
        "use a different name or remove one of the declarations"
    },
    {
        ERROR_VARIABLE_NOT_INITIALIZED,
        WARNING,
        "variable used before being assigned",
        "uninitialized variables contain unpredictable values",
        "initialize the variable before use",
        "add an initializer when declaring the variable"
    },
    {
        ERROR_INVALID_VARIABLE_NAME,
        ERROR,
        "invalid variable name",
        "variable names must follow naming conventions",
        "invalid identifier",
        "use letters, digits, and underscores; start with letter or underscore"
    },

    // Syntax errors (3000s)
    {
        ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS,
        ERROR,
        "invalid float literal",
        "float literals can only contain one decimal point",
        "remove extra decimal points",
        "use a valid float format like `3.14`"
    },
    {
        ERROR_INVALID_FLOAT_INVALID_CHAR,
        ERROR,
        "invalid float literal",
        "float literals can only contain digits and one decimal point",
        "invalid character in float",
        "remove non-numeric characters from the float literal"
    },
    {
        ERROR_INVALID_FLOAT_NO_DIGITS,
        ERROR,
        "invalid float literal",
        "float literals must contain at least one digit",
        "empty float literal",
        "add digits before or after the decimal point"
    },
    {
        ERROR_MISSING_QUOTE,
        ERROR,
        "unterminated string literal",
        "string literals must be enclosed in double quotes",
        "add the missing closing quote",
        "ensure all strings end with `\"`"
    },
    {
        ERROR_INVALID_EXPRESSION,
        ERROR,
        "invalid expression",
        "this expression cannot be parsed correctly",
        "check the syntax",
        "verify operator precedence and parentheses"
    },
    {
        ERROR_UNMATCHED_LEFT_BRACE,
        ERROR,
        "unmatched opening brace",
        "every opening brace needs a corresponding closing brace",
        "missing closing brace",
        "add a closing `}` or remove the extra opening brace"
    },
    {
        ERROR_TERNARY_MISSING_TRUE_BRANCH,
        ERROR,
        "incomplete ternary expression",
        "ternary operator requires both true and false branches",
        "missing expression after `?`",
        "add the true branch: `condition ? true_value : false_value`"
    },
    {
        ERROR_TERNARY_MISSING_FALSE_BRANCH,
        ERROR,
        "incomplete ternary expression",
        "ternary operator requires both true and false branches",
        "missing expression after `:`",
        "add the false branch: `condition ? true_value : false_value`"
    },
    {
        ERROR_TERNARY_INVALID_CONDITION,
        ERROR,
        "invalid ternary condition",
        "ternary condition must be a boolean expression",
        "condition is not boolean",
        "use a comparison or boolean expression before `?`"
    },

    // Logic/Flow errors (4000s)
    {
        ERROR_INVALID_ASSIGNMENT_TARGET,
        ERROR,
        "invalid assignment target",
        "the left side of assignment must be a variable",
        "assignment requires a variable on the left side",
        "assign to a variable, not an expression"
    },
    {
        ERROR_INVALID_OPERATION_FOR_TYPE,
        ERROR,
        "invalid operation for type",
        "this operation is not supported for the given type",
        "operation not available for this type",
        "check which operations are valid for this data type"
    },
    {
        ERROR_INCOMPATIBLE_OPERAND_TYPES,
        ERROR,
        "incompatible operand types",
        "operator cannot be applied to operands of different types",
        "type mismatch in operation",
        "ensure both operands are the same compatible type"
    },
    {
        ERROR_INVALID_UNARY_OPERAND,
        ERROR,
        "invalid unary operand",
        "unary operator cannot be applied to this type",
        "unary operation not supported",
        "check if the unary operator is valid for this data type"
    },

    // System/Internal errors (5000s)
    {
        ERROR_MEMORY_ALLOCATION_FAILED,
        FATAL,
        "memory allocation failed",
        "the system ran out of available memory",
        "insufficient memory",
        "try closing other programs or simplifying the code"
    },
    {
        ERROR_SYMBOL_TABLE_CREATION_FAILED,
        FATAL,
        "symbol table creation failed",
        "unable to create or initialize the symbol table",
        "internal symbol table error",
        "restart the compiler or report this issue"
    },
    {
        ERROR_CONTEXT_CREATION_FAILED,
        FATAL,
        "context creation failed",
        "unable to create parsing or execution context",
        "internal context error",
        "restart the compiler or report this issue"
    },
    {
        ERROR_INTERNAL_PARSER_ERROR,
        FATAL,
        "internal parser error",
        "an unexpected error occurred during parsing",
        "compiler bug detected",
        "please report this issue with the problematic code"
    },
    {
        ERROR_INTERNAL_CODE_GENERATOR_ERROR,
        FATAL,
        "internal code generator error",
        "an unexpected error occurred during code generation",
        "compiler bug in code generation",
        "please report this issue with the problematic code"
    },

    // Sentinel
    {
        ERROR_OK,
        ERROR,
        "unknown error",
        "an unspecified error occurred",
        "error details unavailable",
        "check the compiler output"
    }
};

const ErrorInfo *getErrorInfo(ErrorCode code);
char *formatErrorCode(ErrorCode code);
void reportError(ErrorCode code, ErrorContext *context, const char *extraContext);
void printErrorSummary(void);
void printSourceSnippet(ErrorContext *context);
int hasErrors(void);
int hasFatalErrors(void);
int getErrorCount(void);
int getWarningCount(void);
int getFatalCount(void);
void resetErrorCount(void);
void repError(ErrorCode code, const char *extraContext);

#endif
