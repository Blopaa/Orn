//
// Created by pablo on 31/08/2025.
//

#ifndef UNTITLED22_ERRORHANDLING_H
#define UNTITLED22_ERRORHANDLING_H


// 1000S type errors
// 2000s var errors
// 3000s syntax errors
// 4000s logic errors
// 5000s system errors
typedef enum {
    ERROR_OK = 0,
    ERROR_TYPE_MISMATCH_STRING_TO_INT = 1001,
    ERROR_TYPE_MISMATCH_INT_TO_STRING = 1002,
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
    {ERROR_TYPE_MISMATCH_STRING_TO_INT, ERROR, "Cannot assign string literal to int var"},
    {ERROR_TYPE_MISMATCH_INT_TO_STRING, ERROR, "Cannot assign int literal to string var"},
    {ERROR_OK, ERROR, "Unknown error"}
};

static int errorCount = 0;
static int warningCount = 0;
static int fatalCount = 0;

const ErrorEntry* getErrorEntry(ErrorCode code);
void repError(ErrorCode code, const char* context);
void printErrorSummary(void);
int hasErrors(void);
int hasFatalErrors(void);


#endif