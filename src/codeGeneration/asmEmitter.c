//
// Created by pablo on 10/09/2025.
//

#include "asmTemplate.h"
#include "codeGeneration.h"

void emitFloatTable(StackContext context) {
    if (context->floatDoubleEntries != NULL) {
        ASM_EMIT_SECTION(context->file, ASM_SECTION_RODATA);
        
        FloatDoubleEntry current = context->floatDoubleEntries;
        while (current != NULL) {
            fprintf(context->file, ASM_TEMPLATE_FLOAT_LABEL, current->label);
            fprintf(context->file, ASM_TEMPLATE_FLOAT_DATA, current->value);
            current = current->next;
        }
    }
}

void emitPreamble(StackContext context) {
    if (context == NULL || context->file == NULL) return;

    ASM_EMIT_HEADER_COMMENT(context->file, "Generated code - links with runtime.o");

    emitStringTable(context);
    emitFloatTable(context);

    fprintf(context->file, "\n%s\n", ASM_SECTION_TEXT);
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