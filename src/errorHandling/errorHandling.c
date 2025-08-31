#include "errorHandling.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int errorCount = 0;
static int warningCount = 0;
static int fatalCount = 0;

int getErrorCount(void) {return errorCount;}
int getWarningCount(void) {return warningCount;}
int getFatalCount(void) {return fatalCount;}
void resetErrorCount(void) {
    errorCount = 0;
    warningCount = 0;
    fatalCount = 0;
}

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

int hasErrors(void) {
    return (errorCount > 0 || fatalCount > 0);
}

int hasFatalErrors(void) {
    return (fatalCount > 0);
}
