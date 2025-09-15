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

void repError(ErrorCode code, const char *extraContext) {
	reportError(code, NULL, extraContext);
}

char * formatErrorCode(ErrorCode err) {
	static char buffer[8];
	snprintf(buffer, sizeof(buffer), "E%04d", err);
	return buffer;
}

const ErrorInfo * getErrorInfo(ErrorCode err) {
	for (int i = 0; errorDatabase[i].code != ERROR_OK || i == 0; i++) {
		if (errorDatabase[i].code == err) {
			return &errorDatabase[i];
		}
	}
	return &errorDatabase[sizeof(errorDatabase)/sizeof(errorDatabase[0]) - 1];
}

void printSourceSnippet (ErrorContext * context) {
	if (!context || !context->source) return;
	const char *RED_COLOR = RED;
	const char *RESET_COLOR = RESET;
	printf("%s%4zu |%s %s\n", GRAY, context->line, RESET_COLOR, context->source);
	printf("%s     |%s ", GRAY, RESET_COLOR);
	for (size_t i = 0; i < context->startColumn - 1; i++) {
		printf(" ");
	}

	// Print carets for error length
	printf("%s", RED_COLOR);
	for (size_t i = 0; i < (context->length > 0 ? context->length : 1); i++) {
		printf("^");
	}
	printf("%s\n", RESET_COLOR);
}

void reportError(ErrorCode code, ErrorContext *context, const char *extraContext) {
    const ErrorInfo *info = getErrorInfo(code);
    const char *levelColor, *levelText;

    // Determine colors and text based on error level
    switch (info->level) {
        case WARNING:
            warningCount++;
            levelColor = YELLOW;
            levelText = "warning";
            break;
        case ERROR:
            errorCount++;
            levelColor = RED;
            levelText = "error";
            break;
        case FATAL:
            fatalCount++;
            levelColor = RED ;
            levelText = "error";
            break;
    }

    const char *RESET_COLOR = RESET ;
    const char *BLUE_COLOR = BLUE;

    // Print main error line: error[E1002]: mismatched types
    printf("%s%s %s[%s]:%s %s%s",
           levelColor, levelText, RED,
           formatErrorCode(code),
           RESET_COLOR,
           YELLOW, info->message);

    if (extraContext) {
        printf(" (%s)", extraContext);
    }
    printf("\n");
    if (context && context->file) {
        printf("%s  --> %s:%zu:%zu%s\n",
               YELLOW, context->file, context->line, context->column, RESET_COLOR);
        printf("%s   |%s\n", GRAY, RESET_COLOR);
        printSourceSnippet(context);
        printf("%s   |%s\n", GRAY, RESET_COLOR);
    }

    if (info->help) {
        printf("%s   = help:%s %s\n", BLUE_COLOR, GRAY, info->help);
    }

    if (info->note) {
        printf("%s   = note:%s %s\n", BLUE_COLOR, GRAY, info->note);
    }

    if (info->suggestion) {
        printf("%s   = suggestion:%s %s\n", BLUE_COLOR, GRAY, info->suggestion);
    }

    printf("\n");

    if (info->level == FATAL) {
        printf("%serror:%s could not compile due to fatal error\n",
               levelColor, RESET_COLOR);
        exit(code);
    }
}

void printErrorSummary(void) {
	int totalIssues = warningCount + errorCount + fatalCount;

	if (totalIssues == 0) {
		const char *GREEN_COLOR = GREEN;
		const char *RESET_COLOR = RESET;
		printf("%s✓ Compilation successful:%s No errors or warnings.\n",
			   GREEN_COLOR, RESET_COLOR);
		return;
	}

	const char *RED_COLOR = RED;
	const char *YELLOW_COLOR = YELLOW;
	const char *RESET_COLOR = RESET;

	// Build summary message
	if (errorCount > 0 || fatalCount > 0) {
		printf("%serror:%s could not compile due to ", RED_COLOR, RESET_COLOR);

		int totalErrors = errorCount + fatalCount;
		printf("%d previous error%s", totalErrors, totalErrors == 1 ? "" : "s");

		if (warningCount > 0) {
			printf("; %s%d warning%s emitted%s",
				   YELLOW_COLOR, warningCount, warningCount == 1 ? "" : "s", RESET_COLOR);
		}
		printf("\n");
	} else if (warningCount > 0) {
		printf("%swarning:%s compilation completed with %d warning%s\n",
			   YELLOW_COLOR, RESET_COLOR, warningCount, warningCount == 1 ? "" : "s");
	}
}

int hasErrors(void) {
	return (errorCount > 0 || fatalCount > 0 || warningCount > 0);
}

int hasFatalErrors(void) {
	return (fatalCount > 0);
}
