//
// Created by pablo on 10/09/2025.
//

#include <stdlib.h>
#include <string.h>

#include "asmTemplate.h"
#include "codeGeneration.h"
#include "errorHandling.h"

/**
 * @brief Adds a string literal to the string table with automatic deduplication.
 *
 * Manages the global string table by checking for existing identical strings
 * and only creating new entries when necessary. This deduplication reduces
 * the size of the generated binary by ensuring each unique string appears
 * only once in the data section.
 *
 * Process:
 * 1. Search existing string table for identical value
 * 2. Return existing entry if found (deduplication)
 * 3. Create new StringEntry structure for new strings
 * 4. Generate unique assembly label
 * 5. Link new entry into string table
 * 6. Update string counter for next allocation
 *
 * @param context Code generation context containing string table
 * @param value String literal value (including surrounding quotes)
 * @return StringEntry for the literal (new or existing) or NULL on error
 *
 * @note String comparison includes quotes, so "hello" and "hello" with
 *       different quote styles would be treated as different strings.
 *       Reports ERROR_MEMORY_ALLOCATION_FAILED for allocation failures.
 */
StringEntry addStringLiteral(StackContext context, const char *value) {
    if (context == NULL || value == NULL) return NULL;

    // Check if string already exists
    StringEntry current = context->string;
    while (current != NULL) {
        if (strcmp(current->value, value) == 0) {
            return current;
        }
        current = current->next;
    }

    // Create new string entry
    StringEntry entry = malloc(sizeof(struct StringEntry));
    if (entry == NULL) {
        repError(ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate string entry");
        return NULL;
    }

    entry->value = strdup(value);
    entry->index = context->stringCount++;

    // Create label
    entry->label = malloc(32);
    snprintf(entry->label, 32, "%s%d", ASM_LABEL_PREFIX_STR, entry->index);

    entry->next = context->string;
    context->string = entry;

    return entry;
}

/**
 * @brief Emits the complete string table in the assembly .rodata section.
 *
 * Generates the read-only data section containing all string literals
 * collected during code generation. Handles proper section switching,
 * quote removal, and assembly directive formatting for each string.
 *
 * Process:
 * 1. Check if any strings exist in the table
 * 2. Emit .rodata section directive
 * 3. For each string literal:
 *    a. Remove surrounding quotes for assembly compatibility
 *    b. Emit assembly label
 *    c. Emit .string directive with cleaned value
 * 4. Return to .text section for code emission
 *
 * @param context Code generation context containing string table
 *
 * @note String literals are stored in the .rodata (read-only data) section
 *       which is typically mapped as read-only in the final executable.
 *       Quote removal handles the conversion from source code string format
 *       to assembly string format.
 *
 * Example output:
 * ```
 * .section .rodata
 * .STR_0:
 *     .string "Hello, World!"
 * .STR_1:
 *     .string "Another string"
 * .text
 * ```
 */
void emitStringTable(StackContext context) {
    if (context == NULL || context->file == NULL) return;

    if (context->string != NULL) {
        fprintf(context->file, "\n");
        ASM_EMIT_SECTION(context->file, ASM_SECTION_RODATA);

        StringEntry current = context->string;
        while (current != NULL) {
            // Remove quotes from string value for assembly
            char *cleanValue = strdup(current->value);
            if (cleanValue[0] == '"' && cleanValue[strlen(cleanValue) - 1] == '"') {
                cleanValue[strlen(cleanValue) - 1] = '\0';
                memmove(cleanValue, cleanValue + 1, strlen(cleanValue));
            }

            fprintf(context->file, ASM_TEMPLATE_STRING_LABEL, current->label);
            fprintf(context->file, ASM_TEMPLATE_STRING_DATA, cleanValue);

            free(cleanValue);
            current = current->next;
        }
        fprintf(context->file, "\n");
        ASM_EMIT_SECTION(context->file, ASM_SECTION_TEXT);
    }
}
