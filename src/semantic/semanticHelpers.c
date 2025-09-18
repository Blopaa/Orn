//
// Created by pablo on 15/09/2025.
//

#include "semanticHelpers.h"


#include <stdlib.h>
#include <string.h>

static char *extractSourceLine(const char *source, int lineNum) {
    if (!source || lineNum <= 0) return NULL;

    const char *lineStart = source;
    int currentLine = 1;

    // Find the start of the requested line
    while (*lineStart && currentLine < lineNum) {
        if (*lineStart == '\n') {
            currentLine++;
        }
        lineStart++;
    }

    if (*lineStart == '\0') return NULL;

    // Find the end of the line
    const char *lineEnd = lineStart;
    while (*lineEnd && *lineEnd != '\n') {
        lineEnd++;
    }

    // Create the line string
    size_t lineLength = lineEnd - lineStart;
    char *line = malloc(lineLength + 1);
    if (line) {
        strncpy(line, lineStart, lineLength);
        line[lineLength] = '\0';
    }

    return line;
}

ErrorContext *createErrorContextFromType(ASTNode node, TypeCheckContext context) {
    static ErrorContext errorContext;
    static char *lastSourceLine = NULL;

    if (!node || !context) return NULL;

    // Free previous source line
    if (lastSourceLine) {
        free(lastSourceLine);
        lastSourceLine = NULL;
    }

    // Extract the actual source line
    if (context->sourceFile) {
        lastSourceLine = extractSourceLine(context->sourceFile, node->line);
    }

    errorContext.file = context->filename ? context->filename : "source";
    errorContext.line = node->line;
    errorContext.column = node->column;
    errorContext.source = lastSourceLine;
    errorContext.length = node->length;
    errorContext.startColumn = node->column;

    return &errorContext;
}