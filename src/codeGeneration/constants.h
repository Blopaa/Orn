/**
 * @file constants.h
 * @brief Centralized constants for code generation
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

// ========== STACK LAYOUT ==========

/** Stack offset for first temporary variable (-8(%rbp)) */
#define TEMP_VAR_A_OFFSET 8

/** Stack offset for second temporary variable (-16(%rbp)) */
#define TEMP_VAR_B_OFFSET 16

/** Initial stack frame allocation for temp variables */
#define INITIAL_STACK_FRAME_SIZE 16

// ========== LABEL BUFFER SIZES ==========

/** Max length for assembly labels: ".L<prefix>_<number>" */
#define LABEL_BUFFER_SIZE 32

/** Max length for string labels: ".STR_<number>" */
#define STRING_LABEL_BUFFER_SIZE 32

/** Max length for float/double labels: ".FLOAT_<number>" or ".DOUBLE_<number>" */
#define FLOAT_LABEL_BUFFER_SIZE 32

/** Max length for temporary inline labels */
#define TEMP_LABEL_BUFFER_SIZE 64

// ========== PARSER BUFFER SIZES ==========

/** Buffer size for extracted AST node text */
#define AST_TEXT_BUFFER_SIZE 256

/** Maximum nesting depth for parsing */
#define MAX_PARSE_DEPTH 256

// ========== REGISTER LIMITS ==========

/** Max general-purpose registers: RAX, RBX, RCX, RDX, RSI, RDI, R8-R11 */
#define MAX_GP_REGISTERS 10

/** Max XMM registers for floats: XMM0-XMM5 */
#define MAX_XMM_REGISTERS 6

/** Max function parameters in registers (x86-64 System V ABI) */
#define MAX_REGISTER_PARAMS 6

// ========== ALIGNMENT ==========

/** Standard stack alignment for x86-64 */
#define STACK_ALIGNMENT 16

/** Minimum alignment for any data type */
#define MIN_ALIGNMENT 8

// ========== ERROR HANDLING ==========

/** Max length for error code strings: "E<4-digit>" */
#define ERROR_CODE_BUFFER_SIZE 8

/** Max extracted source line length for errors */
#define ERROR_SOURCE_LINE_MAX 512

// ========== SYMBOL TABLE ==========

/** Initial capacity for symbol table */
#define SYMBOL_TABLE_INITIAL_SIZE 256

/** Maximum identifier/symbol name length */
#define MAX_SYMBOL_NAME_LENGTH 256

#endif // CONSTANTS_H