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
    if(!node || !context) return NULL;
    ErrorContext * errCtx = malloc(sizeof(ErrorContext));
    if(!errCtx) return NULL;
    char * sourceLine = NULL;
    if(context->sourceFile){
        sourceLine = extractSourceLine(context->sourceFile, node->line);
    }
    errCtx->file = context->filename ? context->filename : "source";
    errCtx->line = node->line;
    errCtx->column = node->column;
    errCtx->source = sourceLine;
    errCtx->length = node->length;
    errCtx->startColumn = node->column;
    return errCtx;
}

void freeErrorContext(ErrorContext * errCtx){
    if(errCtx){
        free(errCtx->source);
        free(errCtx);
    }
}