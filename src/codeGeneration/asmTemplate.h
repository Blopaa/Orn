/**
 * @file asmTemplates.h
 * @brief Centralized assembly code templates and mnemonics for code generation.
 *
 * Contains all hardcoded assembly strings, templates, and mnemonics used
 * throughout the code generation process. Centralizing these strings improves
 * maintainability, consistency, and makes it easier to adapt to different
 * assembly dialects or architectures.
 */

#ifndef ASM_TEMPLATES_H
#define ASM_TEMPLATES_H

// ========== ASSEMBLY SECTIONS ==========
#define ASM_SECTION_TEXT        ".text"
#define ASM_SECTION_RODATA      ".section .rodata"
#define ASM_SECTION_DATA        ".data"
#define ASM_SECTION_BSS         ".bss"

// ========== PROGRAM STRUCTURE ==========
#define ASM_GLOBAL_START        ".globl _start"
#define ASM_START_LABEL         "_start:"
//16 is for two temp vars of 8bytes
#define ASM_FUNCTION_PROLOGUE   "    pushq %rbp\n    movq %rsp, %rbp\n    subq $16, %rsp"
#define ASM_FUNCTION_EPILOGUE   "    movq %rbp, %rsp\n    popq %rbp"

// ========== SYSTEM CALLS ==========
#define ASM_SYS_EXIT            "    movq $60, %%rax     # sys_exit"
#define ASM_EXIT_STATUS_SUCCESS "    movq $0, %%rdi      # exit status"
#define ASM_SYSCALL             "    syscall"

// ========== GENERAL PURPOSE REGISTER STRINGS ==========
#define ASM_REG_RAX             "%rax"
#define ASM_REG_RBX             "%rbx"
#define ASM_REG_RCX             "%rcx"
#define ASM_REG_RDX             "%rdx"
#define ASM_REG_RSI             "%rsi"
#define ASM_REG_RDI             "%rdi"
#define ASM_REG_RSP             "%rsp"
#define ASM_REG_RBP             "%rbp"
#define ASM_REG_R8              "%r8"
#define ASM_REG_R9              "%r9"
#define ASM_REG_R10             "%r10"
#define ASM_REG_R11             "%r11"

// ========== SSE REGISTER STRINGS ==========
#define ASM_REG_XMM0            "%xmm0"
#define ASM_REG_XMM1            "%xmm1"
#define ASM_REG_XMM2            "%xmm2"
#define ASM_REG_XMM3            "%xmm3"
#define ASM_REG_XMM4            "%xmm4"
#define ASM_REG_XMM5            "%xmm5"

// ========== 8-BIT REGISTER STRINGS ==========
#define ASM_REG_AL              "%al"

// ========== MOVE INSTRUCTIONS ==========
#define ASM_MOVQ                "movq"
#define ASM_MOVSD               "movsd"
#define ASM_MOVZBQ              "movzbq"
#define ASM_LEAQ                "leaq"

// Float register move template
#define ASM_TEMPLATE_MOVSD_REG_REG      "    movsd %s, %s\n"

// ========== ARITHMETIC INSTRUCTIONS ==========
#define ASM_ADDQ                "addq"
#define ASM_ADDSD               "addsd"
#define ASM_SUBQ                "subq"
#define ASM_SUBSD               "subsd"
#define ASM_IMULQ               "imulq"
#define ASM_MULSD               "mulsd"
#define ASM_IDIVQ               "idivq"
#define ASM_DIVSD               "divsd"
#define ASM_NEGQ                "negq"
#define ASM_INCQ                "incq"
#define ASM_DECQ                "decq"
#define ASM_CQO                 "cqo"

// ========== LOGICAL INSTRUCTIONS ==========
#define ASM_ANDQ                "andq"
#define ASM_ORQ                 "orq"
#define ASM_XORPD               "xorpd"
#define ASM_TESTQ               "testq"

// ========== COMPARISON INSTRUCTIONS ==========
#define ASM_CMPQ                "cmpq"
#define ASM_UCOMISD             "ucomisd"

// ========== JUMP INSTRUCTIONS ==========
#define ASM_JMP                 "jmp"
#define ASM_JZ                  "jz"
#define ASM_JE                  "je"
#define ASM_JNE                 "jne"

// ========== SET INSTRUCTIONS ==========
#define ASM_SETE                "sete"
#define ASM_SETNE               "setne"
#define ASM_SETL                "setl"
#define ASM_SETG                "setg"
#define ASM_SETLE               "setle"
#define ASM_SETGE               "setge"
#define ASM_SETB                "setb"
#define ASM_SETA                "seta"
#define ASM_SETBE               "setbe"
#define ASM_SETAE               "setae"
#define ASM_SETZ                "setz"

// ========== STACK INSTRUCTIONS ==========
#define ASM_PUSHQ               "pushq"
#define ASM_POPQ                "popq"
#define ASM_SUBQ_RSP            "subq"

// ========== DATA DIRECTIVES ==========
#define ASM_STRING_DIRECTIVE    ".string"
#define ASM_DOUBLE_DIRECTIVE    ".double"
#define ASM_QUAD_DIRECTIVE      ".quad"

// ========== MEMORY ADDRESSING ==========
#define ASM_RIP_RELATIVE        "(%%rip)"
#define ASM_RBP_OFFSET          "(%%rbp)"

// ========== IMMEDIATE VALUE PREFIXES ==========
#define ASM_IMMEDIATE_PREFIX    "$"

// ========== FLOAT CONSTANTS ==========
#define ASM_FLOAT_SIGN_MASK     "0x8000000000000000"

// ========== INSTRUCTION TEMPLATES ==========
// Basic move operations
#define ASM_TEMPLATE_MOVQ_IMM_REG       "    movq $%s, %s    # %s: %s\n"
#define ASM_TEMPLATE_MOVQ_MEM_REG       "    movq %d(%%rbp), %s    # Load %s\n"
#define ASM_TEMPLATE_MOVQ_REG_MEM       "    movq %s, %d(%%rbp)    # Store %s\n"
#define ASM_TEMPLATE_MOVQ_REG_REG       "    movq %s, %s\n"

// Float operations
#define ASM_TEMPLATE_MOVSD_MEM_REG      "    movsd %d(%%rbp), %s    # Load float %s\n"
#define ASM_TEMPLATE_MOVSD_REG_MEM      "    movsd %s, %d(%%rbp)    # Store float %s\n"
#define ASM_TEMPLATE_MOVSD_REG_REG      "    movsd %s, %s\n"
#define ASM_TEMPLATE_MOVSD_LABEL_REG    "    movsd %s(%%rip), %s    # Load float immediate: %s\n"

// String operations
#define ASM_TEMPLATE_LEAQ_LABEL_REG     "    leaq %s(%%rip), %s    # Load string: %s\n"

// Stack operations
#define ASM_TEMPLATE_SUBQ_RSP           "    subq $%d, %%rsp    # Allocate %s (%d bytes)\n"

// Binary operations
#define ASM_TEMPLATE_BINARY_OP          "    %s %s, %s\n"

// For instructions with RIP-relative addressing
#define ASM_TEMPLATE_XOR_RIP            "    %s %s(%%rip), %s\n"

// For single operand instructions
#define ASM_TEMPLATE_UNARY_OP           "    %s %s\n"

// For test instruction (same operand twice)
#define ASM_TEMPLATE_TEST_SAME          "    %s %s, %s\n"

// For instructions with immediate values and RIP addressing
#define ASM_TEMPLATE_LOAD_RIP           "    %s %s(%%rip), %s\n"

// Comparison operations
#define ASM_TEMPLATE_CMP_REGS           "    cmpq %s, %s\n"
#define ASM_TEMPLATE_CMP_SET            "    %s %%al\n"
#define ASM_TEMPLATE_CMP_EXTEND         "    movzbq %%al, %s\n"

// Float comparison operations
#define ASM_TEMPLATE_FLOAT_CMP          "    ucomisd %s, %s\n"

// Test operations
#define ASM_TEMPLATE_TEST_REG           "    testq %s, %s\n"

// Division setup
#define ASM_TEMPLATE_DIV_SETUP          "    movq %s, %%rax\n    cqo\n    idivq %s\n"
#define ASM_TEMPLATE_DIV_RESULT_QUOT    "    movq %%rax, %s\n"
#define ASM_TEMPLATE_DIV_RESULT_REM     "    movq %%rdx, %s\n"

// Float data declarations
#define ASM_TEMPLATE_FLOAT_LABEL        "%s:\n"
#define ASM_TEMPLATE_FLOAT_DATA         "    .double %s\n"
#define ASM_TEMPLATE_FLOAT_MASK         "    .quad %s\n"

// String data declarations
#define ASM_TEMPLATE_STRING_LABEL       "%s:\n"
#define ASM_TEMPLATE_STRING_DATA        "    .string \"%s\"\n"

// Control flow
#define ASM_TEMPLATE_LABEL              "%s:\n"
#define ASM_TEMPLATE_JUMP               "    jmp %s\n"
#define ASM_TEMPLATE_CONDITIONAL_JUMP   "    %s %s\n"

// Comments
#define ASM_TEMPLATE_COMMENT            "    # %s\n"
#define ASM_TEMPLATE_HEADER_COMMENT     "# %s\n"

// ========== HELPER MACROS ==========
#define ASM_EMIT_COMMENT(file, text) \
    fprintf(file, ASM_TEMPLATE_COMMENT, text)

#define ASM_EMIT_HEADER_COMMENT(file, text) \
    fprintf(file, ASM_TEMPLATE_HEADER_COMMENT, text)

#define ASM_EMIT_LABEL(file, label) \
    fprintf(file, ASM_TEMPLATE_LABEL, label)

#define ASM_EMIT_SECTION(file, section) \
    fprintf(file, "%s\n", section)

#define ASM_EMIT_MOVQ_IMM(file, value, reg, type, comment) \
    fprintf(file, ASM_TEMPLATE_MOVQ_IMM_REG, value, reg, type, comment)

#define ASM_EMIT_MOVQ_MEM_TO_REG(file, offset, reg, varname) \
    fprintf(file, ASM_TEMPLATE_MOVQ_MEM_REG, offset, reg, varname)

#define ASM_EMIT_MOVQ_REG_TO_MEM(file, reg, offset, varname) \
    fprintf(file, ASM_TEMPLATE_MOVQ_REG_MEM, reg, offset, varname)

#define ASM_EMIT_BINARY_OP(file, op, src, dst) \
    fprintf(file, ASM_TEMPLATE_BINARY_OP, op, src, dst)

#define ASM_EMIT_SUBQ_RSP(file, size, varname) \
    fprintf(file, ASM_TEMPLATE_SUBQ_RSP, size, varname, size)

// ========== LABEL PREFIXES ==========
#define ASM_LABEL_PREFIX_STR        ".STR_"
#define ASM_LABEL_PREFIX_FLOAT      ".FLOAT_"
#define ASM_LABEL_PREFIX_FLOAT_NEG  ".FLOAT_NEG_"
#define ASM_LABEL_PREFIX_LOCAL      ".L"
#define ASM_LABEL_PREFIX_LOOP       "loop"
#define ASM_LABEL_PREFIX_END_LOOP   "end_loop"
#define ASM_LABEL_PREFIX_ELSE       "else"
#define ASM_LABEL_PREFIX_END_IF     "end_if"

// ========== BOOLEAN VALUES ==========
#define ASM_BOOL_TRUE_VALUE         1
#define ASM_BOOL_FALSE_VALUE        0

#endif // ASM_TEMPLATES_H