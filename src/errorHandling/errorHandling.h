//
// Created by pablo on 31/08/2025.
//

#ifndef UNTITLED22_ERRORHANDLING_H
#define UNTITLED22_ERRORHANDLING_H


// 2000s var errors
// 4000s logic errors
// 5000s system errors
typedef enum {
    ERROR_OK = 0,
    // 1000S type errors
    ERROR_TYPE_MISMATCH_STRING_TO_INT = 1001,
    ERROR_TYPE_MISMATCH_INT_TO_STRING = 1002,
    ERROR_TYPE_MISMATCH_FLOAT_TO_STRING = 1003,
    ERROR_TYPE_MISMATCH_STRING_TO_FLOAT = 1004,
    ERROR_TYPE_MISMATCH_FLOAT_TO_INT = 1005,
    // 3000s syntax errors
    ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS = 3001,
    ERROR_INVALID_FLOAT_INVALID_CHAR = 3002,
    ERROR_INVALID_FLOAT_NO_DIGITS = 3003,
    ERROR_MISSING_QUOTE = 3004,
    ERROR_INVALID_EXPRESSION = 3005
} ErrorCode;

// WARING -> ERROR -> FATAL
typedef enum {
    WARNING,
    ERROR,
    FATAL
} ErrorLevel;

typedef struct {
    ErrorCode code;
    ErrorLevel level;
    const char *message;
    char *cntxt;
    int line; // for future
} Error;

typedef struct {
    ErrorCode code;
    ErrorLevel level;
    const char *message;
} ErrorEntry;

static const ErrorEntry errorList[] = {
    {ERROR_TYPE_MISMATCH_STRING_TO_INT, ERROR, "Cannot assign string literal to int variable"},
    {ERROR_TYPE_MISMATCH_INT_TO_STRING, ERROR, "Cannot assign int literal to string variable"},
    {ERROR_TYPE_MISMATCH_STRING_TO_FLOAT, ERROR, "Cannot assign string literal to float variable"},
    {ERROR_TYPE_MISMATCH_FLOAT_TO_STRING, ERROR, "Cannot assign float literal to string variable"},
    {ERROR_TYPE_MISMATCH_FLOAT_TO_INT, ERROR, "Cannot assign float literal to int variable"},

    // Syntax errors
    {ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS, ERROR, "Float literal cannot have multiple decimal points"},
    {ERROR_INVALID_FLOAT_INVALID_CHAR, ERROR, "Float literal contains invalid character"},
    {ERROR_INVALID_FLOAT_NO_DIGITS, ERROR, "Float literal must contain at least one digit"},
    {ERROR_INVALID_EXPRESSION, ERROR, "Invalid expression or operator used as value"},
    {ERROR_MISSING_QUOTE, ERROR, "Invalid string expression"},


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
