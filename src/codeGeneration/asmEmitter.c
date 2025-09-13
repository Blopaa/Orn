//
// Created by pablo on 10/09/2025.
//

#include "asmTemplate.h"
#include "codeGeneration.h"

void emitPreamble(StackContext context) {
    if (context == NULL || context->file == NULL) return;

    ASM_EMIT_HEADER_COMMENT(context->file, "Generated code - links with runtime.o");

    emitStringTable(context);

    fprintf(context->file, "\n%s\n", ASM_SECTION_TEXT);

    fprintf(context->file, ".globl main\n");
    fprintf(context->file, "main:\n");
    fprintf(context->file, "%s\n", ASM_FUNCTION_PROLOGUE);
    fprintf(context->file, "\n");
}

void emitEpilogue(StackContext context) {
    if (context == NULL || context->file == NULL) return;

    fprintf(context->file, "\n");
    ASM_EMIT_COMMENT(context->file, "End of main function");
    fprintf(context->file, "%s\n", ASM_FUNCTION_EPILOGUE);
    fprintf(context->file, "    ret                  # Return to runtime\n");
}

/**
 * @brief Emits a formatted comment in the assembly output.
 *
 * Adds descriptive comments to the generated assembly code to improve
 * readability and debugging. Comments are properly formatted with
 * consistent indentation and prefixed with the assembly comment
 * character (#).
 *
 * @param context Code generation context containing output file
 * @param comment Comment text to emit (can be NULL)
 *
 * @note Comments are only emitted if both context and comment are valid.
 *       This function is used throughout code generation to document
 *       the purpose of generated assembly sequences.
 */
void emitComment(StackContext context, const char *comment) {
    if (context && context->file && comment) {
        ASM_EMIT_COMMENT(context->file, comment);
    }
}