/**
* @file errorHandling.c
 * @brief Implementation of the error handling system for the C compiler.
 *
 * Provides error reporting, counting, and summary functionality with
 * support for different severity levels (WARNING, ERROR, FATAL).
 * Maintains global error state and provides formatted error output.
 */

#include "errorHandling.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @internal Global counter for non-fatal errors */
static int errorCount = 0;
/** @internal Global counter for warning messages */
static int warningCount = 0;
/** @internal Global counter for fatal errors */
static int fatalCount = 0;

/**
 * @brief Gets the current count of non-fatal errors.
 *
 * @return Number of ERROR-level issues encountered
 */
int getErrorCount(void) { return errorCount; }

/**
 * @brief Gets the current count of warnings.
 *
 * @return Number of WARNING-level issues encountered
 */
int getWarningCount(void) { return warningCount; }

/**
 * @brief Gets the current count of fatal errors.
 *
 * @return Number of FATAL-level issues encountered
 */
int getFatalCount(void) { return fatalCount; }

/**
 * @brief Resets all error counters to zero.
 *
 * Used primarily for testing to ensure clean state between test cases.
 * Essential for Unity test framework integration where each test
 * should start with a clean error state.
 */
void resetErrorCount(void) {
    errorCount = 0;
    warningCount = 0;
    fatalCount = 0;
}

/**
 * @brief Retrieves error entry information for a given error code.
 *
 * Performs linear search through the static errorList array to find
 * the entry matching the given error code. If no match is found,
 * returns the last entry which serves as the "unknown error" fallback.
 *
 * @param code The error code to look up
 * @return Pointer to the ErrorEntry containing error details
 *
 * @note Time complexity is O(n) where n is the number of error codes.
 *       For better performance with many error codes, consider using
 *       a hash table or binary search with sorted array.
 */
const ErrorEntry *getErrorEntry(ErrorCode code) {
    int i = 0;
    while (errorList[i].code != ERROR_OK) {
        if (errorList[i].code == code) {
            return &errorList[i];
        }
        i++;
    }
    return &errorList[i]; // returns unknown
}

/**
 * @brief Reports an error with optional context information.
 *
 * Looks up error details, increments appropriate counter based on severity,
 * formats and prints the error message with context. For FATAL errors,
 * immediately terminates the program with the error code as exit status.
 *
 * Output format:
 * - "[LEVEL] [code]: message"
 * - "[LEVEL] [code]: message - 'context'" (if context provided)
 * - "compilation aborted due to fatal error." (for FATAL errors)
 *
 * @param code The specific error code to report
 * @param cntxt Optional context string providing additional details (can be NULL)
 *
 * @note FATAL errors will call exit() and terminate the program immediately.
 *       The exit code will be the numeric value of the error code.
 */
void repError(ErrorCode code, const char *cntxt) {
    const ErrorEntry *entry = getErrorEntry(code);
    switch (entry->level) {
        case WARNING:
            warningCount++;
            printf("WARNING ");
            break;
        case ERROR:
            errorCount++;
            printf("ERROR ");
            break;
        case FATAL:
            fatalCount++;
            printf("FATAL ");
            break;
    }

    printf("[%d]: %s", code, entry->message);
    if (cntxt && strlen(cntxt) > 0) {
        printf(" - '%s'", cntxt);
    }
    printf("\n");
    if (entry->level == FATAL) {
        printf("compilation aborted due  to fatal error.\n");
        exit(code);
    }
}

/**
 * @brief Prints a comprehensive summary of compilation errors and warnings.
 *
 * Analyzes error counts and displays either a success message or detailed
 * breakdown of issues encountered during compilation. Provides clear
 * indication of compilation status.
 *
 * Output formats:
 * - Success: "Compilation successful: No errors or warnings."
 * - Failure: Shows counts of warnings, errors, and fatal errors,
 *           followed by "Compilation failed." if any errors occurred.
 */
void printErrorSummary(void) {
    int totalIssues = warningCount + errorCount + fatalCount;

    if (totalIssues == 0) {
        printf("Compilation successful: No errors or warnings.\n");
    } else {
        printf("\nCompilation summary:\n");
        if (warningCount > 0) printf("  %d warning(s)\n", warningCount);
        if (errorCount > 0) printf("  %d error(s)\n", errorCount);
        if (fatalCount > 0) printf("  %d fatal error(s)\n", fatalCount);

        if (errorCount > 0 || fatalCount > 0) {
            printf("Compilation failed.\n");
        }
    }
}

/**
 * @brief Checks if any errors or fatal errors occurred during compilation.
 *
 * Used to determine if compilation should be considered failed.
 * Warnings alone do not constitute compilation failure.
 *
 * @return 1 if there are errors or fatal errors, 0 otherwise
 *
 * @note This function is commonly used as a return value for main()
 *       to indicate process exit status to the operating system.
 */
int hasErrors(void) {
    return (errorCount > 0 || fatalCount > 0 || warningCount > 0);
}

/**
 * @brief Checks if any fatal errors occurred during compilation.
 *
 * Fatal errors indicate catastrophic failures that prevent any
 * meaningful continuation of compilation. This is typically used
 * for early termination checks.
 *
 * @return 1 if there are fatal errors, 0 otherwise
 */
int hasFatalErrors(void) {
    return (fatalCount > 0);
}
