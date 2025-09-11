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

/**
 * @brief Detailed error information structure.
 *
 * Contains comprehensive error context including location,
 * message, and additional context information.
 * Used for future enhancement of error reporting.
 */
typedef struct {
    ErrorCode code;
    ErrorLevel level;
    const char *message;
    char *cntxt;
    int line; // for future
} Error;

/**
 * @brief Static error definition entry.
 *
 * Maps error codes to their severity levels and messages.
 * Used in the static error lookup table for efficient error reporting.
 */
typedef struct {
    ErrorCode code;
    ErrorLevel level;
    const char *message;
} ErrorEntry;

/**
 * @brief Static lookup table mapping error codes to error information.
 *
 * Provides O(n) lookup for error details. The array is terminated
 * with an ERROR_OK entry that serves as the "unknown error" fallback.
 */
static const ErrorEntry errorList[] = {
    {ERROR_TYPE_MISMATCH_STRING_TO_INT, ERROR, "Cannot assign string literal to int variable"},
    {ERROR_TYPE_MISMATCH_INT_TO_STRING, ERROR, "Cannot assign int literal to string variable"},
    {ERROR_TYPE_MISMATCH_STRING_TO_FLOAT, ERROR, "Cannot assign string literal to float variable"},
    {ERROR_TYPE_MISMATCH_FLOAT_TO_STRING, ERROR, "Cannot assign float literal to string variable"},
    {ERROR_TYPE_MISMATCH_FLOAT_TO_INT, ERROR, "Cannot assign float literal to int variable"},
    {ERROR_TYPE_MISMATCH_INT_TO_BOOL, ERROR, "Cannot assign int literal to bool variable"},
    {ERROR_TYPE_MISMATCH_BOOL_TO_STRING, ERROR, "Cannot assign bool literal to string variable"},
    {ERROR_TYPE_MISMATCH_BOOL_TO_INT, ERROR, "Cannot assign bool literal to int variable"},
    {ERROR_TYPE_MISMATCH_BOOL_TO_FLOAT, ERROR, "Cannot assign bool literal to float variable"},

    // Syntax errors
    {ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS, ERROR, "Float literal cannot have multiple decimal points"},
    {ERROR_INVALID_FLOAT_INVALID_CHAR, ERROR, "Float literal contains invalid character"},
    {ERROR_INVALID_FLOAT_NO_DIGITS, ERROR, "Float literal must contain at least one digit"},
    {ERROR_INVALID_EXPRESSION, ERROR, "Invalid expression or operator used as value"},
    {ERROR_MISSING_QUOTE, ERROR, "Invalid string expression"},
    {ERROR_TERNARY_MISSING_TRUE_BRANCH, ERROR, "Expected expression or block after '?' in ternary"},
    {ERROR_TERNARY_MISSING_FALSE_BRANCH, ERROR, "Expected expression or block after ':' in ternary"},
    {ERROR_TERNARY_INVALID_CONDITION, ERROR, "Invalid condition in ternary expression"},

    // Variable errors
    {ERROR_UNDEFINED_VARIABLE, ERROR, "Undefined variable"},
    {ERROR_VARIABLE_REDECLARED, ERROR, "Variable already declared in current scope"},
    {ERROR_VARIABLE_NOT_INITIALIZED, WARNING, "Variable used before initialization"},
    {ERROR_INVALID_VARIABLE_NAME, ERROR, "Invalid variable name"},

    // Logic errors
    {ERROR_INVALID_ASSIGNMENT_TARGET, ERROR, "Invalid assignment target"},
    {ERROR_INVALID_OPERATION_FOR_TYPE, ERROR, "Operation not valid for this type"},
    {ERROR_INCOMPATIBLE_OPERAND_TYPES, ERROR, "Operand types are not compatible"},
    {ERROR_INVALID_UNARY_OPERAND, ERROR, "Invalid operand for unary operation"},

    // System errors
    {ERROR_MEMORY_ALLOCATION_FAILED, FATAL, "Memory allocation failed"},
    {ERROR_SYMBOL_TABLE_CREATION_FAILED, FATAL, "Failed to create symbol table"},
    {ERROR_CONTEXT_CREATION_FAILED, FATAL, "Failed to create type checking context"},
    {ERROR_INTERNAL_PARSER_ERROR, FATAL, "Internal parser error"},
    {ERROR_INTERNAL_CODE_GENERATOR_ERROR, FATAL, "Internal code generation error"},

    {ERROR_OK, ERROR, "Unknown error"}
};

const ErrorEntry *getErrorEntry(ErrorCode code);

void repError(ErrorCode code, const char *context);

void printErrorSummary(void);

int hasErrors(void);

int hasFatalErrors(void);

int getErrorCount(void);

int getWarningCount(void);

int getFatalCount(void);

void resetErrorCount(void);

#endif
