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

// this is only for /semantic
#define REPORT_ERROR(code, node, ctx, msg) \
    do { \
        ErrorContext *_ectx = createErrorContextFromType((node), (ctx)); \
        reportError((code), _ectx, (msg)); \
        freeErrorContext(_ectx); \
    } while(0)

/**
 * @brief Error code enumeration categorized by type.
 *
 * Error codes are organized in ranges:
 * - 1000s: Type mismatch errors
 * - 2000s: Variable/Symbol errors
 * - 3000s: Syntax errors
 * - 4000s: Logic/Control flow errors
 * - 5000s: Function-related errors
 * - 6000s: System/Internal errors
 */
typedef enum {
	ERROR_OK = 0,
	// 1000s: Type mismatch errors
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
	ERROR_INCOMPATIBLE_BINARY_OPERANDS = 1012,
	ERROR_VOID_IN_EXPRESSION = 1013,
	ERROR_TYPE_MISMATCH_DOUBLE_TO_INT = 1014,
	ERROR_TYPE_MISMATCH_DOUBLE_TO_FLOAT = 1015,
	ERROR_TYPE_MISMATCH_STRING_TO_DOUBLE = 1016,
	ERROR_TYPE_MISMATCH_BOOL_TO_DOUBLE = 1017,
	ERROR_TYPE_MISMATCH_DOUBLE_TO_BOOL = 1018,
	ERROR_TYPE_MISMATCH_DOUBLE_TO_STRING = 1019,

	// 2000s: Variable/Symbol errors
	ERROR_UNDEFINED_VARIABLE = 2001,
	ERROR_VARIABLE_REDECLARED = 2002,
	ERROR_VARIABLE_NOT_INITIALIZED = 2003,
	ERROR_INVALID_VARIABLE_NAME = 2004,
	ERROR_CONSTANT_REASSIGNMENT = 2005,
	ERROR_UNDEFINED_SYMBOL = 2006,
	ERROR_SYMBOL_NOT_VARIABLE = 2007,

	// 3000s: Syntax errors
	ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS = 3001,
	ERROR_INVALID_FLOAT_INVALID_CHAR = 3002,
	ERROR_INVALID_FLOAT_NO_DIGITS = 3003,
	ERROR_EXPECTED_QUOTE = 3004,
	ERROR_INVALID_EXPRESSION = 3005,
	ERROR_UNMATCHED_LEFT_BRACE = 3006,
	ERROR_TERNARY_MISSING_TRUE_BRANCH = 3007,
	ERROR_TERNARY_MISSING_FALSE_BRANCH = 3008,
	ERROR_TERNARY_INVALID_CONDITION = 3009,
	ERROR_EXPECTED_SEMICOLON = 3010,
	ERROR_UNEXPECTED_TOKEN = 3011,
	ERROR_EXPECTED_CLOSING_PAREN = 3012,
	ERROR_EXPECTED_OPENING_PAREN = 3013,
	ERROR_EXPECTED_OPENING_BRACE = 3014,
	ERROR_EXPECTED_CLOSING_BRACE = 3015,
	ERROR_EXPECTED_QUESTION_MARK = 3016,
	ERROR_EXPECTED_COLON = 3017,
	ERROR_EXPECTED_ARROW = 3018,
	ERROR_EXPECTED_RETURN = 3019,
	ERROR_EXPECTED_FN = 3020,
	ERROR_CONST_MUST_BE_INITIALIZED = 3021,

	// 4000s: Logic/Control flow errors
	ERROR_INVALID_ASSIGNMENT_TARGET = 4001,
	ERROR_INVALID_OPERATION_FOR_TYPE = 4002,
	ERROR_INCOMPATIBLE_OPERAND_TYPES = 4003,
	ERROR_INVALID_UNARY_OPERAND = 4004,
	ERROR_UNREACHABLE_CODE = 4005,
	ERROR_BREAK_OUTSIDE_LOOP = 4006,
	ERROR_CONTINUE_OUTSIDE_LOOP = 4007,
	ERROR_INVALID_LOOP_CONDITION = 4008,
	ERROR_INVALID_CAST_TARGET = 4009,
	ERROR_FORBIDDEN_CAST = 4010,
	ERROR_CAST_PRECISION_LOSS = 4011,

	// 5000s: Function-related errors
	ERROR_FUNCTION_REDEFINED = 5001,
	ERROR_UNDEFINED_FUNCTION = 5002,
	ERROR_FUNCTION_ARG_COUNT_MISMATCH = 5003,
	ERROR_FUNCTION_ARG_TYPE_MISMATCH = 5004,
	ERROR_MISSING_RETURN_VALUE = 5005,
	ERROR_UNEXPECTED_RETURN_VALUE = 5006,
	ERROR_RETURN_TYPE_MISMATCH = 5007,
	ERROR_INVALID_FUNCTION_NAME = 5008,
	ERROR_TOO_MANY_PARAMETERS = 5009,
	ERROR_DUPLICATE_PARAMETER_NAME = 5010,
	ERROR_INVALID_PARAMETER_TYPE = 5011,
	ERROR_CALLING_NON_FUNCTION = 5012,
	ERROR_FUNCTION_NO_OVERLOAD_MATCH = 5013,

	// 6000s: System/Internal errors
	ERROR_MEMORY_ALLOCATION_FAILED = 6001,
	ERROR_SYMBOL_TABLE_CREATION_FAILED = 6002,
	ERROR_CONTEXT_CREATION_FAILED = 6003,
	ERROR_INTERNAL_PARSER_ERROR = 6004,
	ERROR_INTERNAL_CODE_GENERATOR_ERROR = 6005,
	ERROR_FILE_NOT_FOUND = 6006,
	ERROR_FILE_READ_ERROR = 6007,
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
	char* source;
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

extern const ErrorInfo errorDatabase[];
extern const size_t errorDatabaseCount;

const ErrorInfo *getErrorInfo(ErrorCode code);
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
