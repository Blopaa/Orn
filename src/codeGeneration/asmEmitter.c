//
// Created by pablo on 10/09/2025.
//

#include "codeGeneration.h"

/**
 * @brief Emits the assembly program preamble with setup code.
 *
 * Generates the initial assembly code structure including:
 * - String table emission (must come before text section)
 * - Global start symbol declaration
 * - Entry point setup with stack frame initialization
 * - Standard function prologue for main program
 *
 * The preamble establishes the execution environment and prepares
 * the stack for variable allocation and function calls.
 *
 * @param context Code generation context containing output file
 *
 * @note Emits x86-64 assembly compatible with GNU assembler.
 *       The string table is emitted first to ensure proper section
 *       ordering in the final executable.
 */
void emitPreamble(StackContext context) {
    if (context == NULL || context->file == NULL) return;

    fprintf(context->file, "# Generated assembly code\n");

    // FIX: Emit string table before text section
    emitStringTable(context);

    fprintf(context->file, ".globl _start\n");
    fprintf(context->file, "\n");
    fprintf(context->file, "_start:\n");
    fprintf(context->file, "    pushq %%rbp\n");
    fprintf(context->file, "    movq %%rsp, %%rbp\n");
    fprintf(context->file, "\n");
}

/**
 * @brief Emits the assembly program epilogue with exit code.
 *
 * Generates the program termination sequence including:
 * - Stack frame cleanup (restore rbp and rsp)
 * - System call setup for program exit
 * - Clean exit with status code 0
 *
 * Uses the Linux x86-64 system call convention (sys_exit = 60)
 * to terminate the program gracefully.
 *
 * @param context Code generation context containing output file
 *
 * @note The exit status is hardcoded to 0 (success). Future
 *       enhancements could allow configurable exit codes based
 *       on program execution results.
 */
void emitEpilogue(StackContext context) {
    if (context == NULL || context->file == NULL) return;

    fprintf(context->file, "\n");
    emitComment(context, "Exit program");
    fprintf(context->file, "    movq %%rbp, %%rsp\n");
    fprintf(context->file, "    popq %%rbp\n");
    fprintf(context->file, "    movq $60, %%rax     # sys_exit\n");
    fprintf(context->file, "    movq $0, %%rdi      # exit status\n");
    fprintf(context->file, "    syscall\n");
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
        fprintf(context->file, "    # %s\n", comment);
    }
}