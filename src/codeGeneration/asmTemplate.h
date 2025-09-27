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

// Generic move/arithmetic with suffix
#define ASM_TEMPLATE_MOV_REG_REG        "    mov%s %s, %s\n"
#define ASM_TEMPLATE_ADD_REG_REG        "    add%s %s, %s\n"
#define ASM_TEMPLATE_SUB_REG_REG        "    sub%s %s, %s\n"
#define ASM_TEMPLATE_IMUL_REG_REG       "    imul%s %s, %s\n"
#define ASM_TEMPLATE_AND_REG_REG        "    and%s %s, %s\n"
#define ASM_TEMPLATE_OR_REG_REG         "    or%s %s, %s\n"
#define ASM_TEMPLATE_CMP_SUFFIX         "    cmp%s %s, %s\n"
#define ASM_TEMPLATE_NEG_SUFFIX         "    neg%s %s    # Invert result\n"
#define ASM_TEMPLATE_LOAD_INT           "    mov%s %d(%%rbp), %s    # Load int %s\n"
#define ASM_TEMPLATE_RET                "    ret\n"

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

// Float operations
#define ASM_LABEL_PREFIX_DOUBLE     ".DOUBLE_"
#define ASM_TEMPLATE_MOVSD_MEM_REG      "    movsd %d(%%rbp), %s    # Load float %s\n"
#define ASM_TEMPLATE_MOVSD_REG_MEM      "    movsd %s, %d(%%rbp)    # Store float %s\n"
#define ASM_TEMPLATE_MOVSD_REG_REG      "    movsd %s, %s\n"
#define ASM_TEMPLATE_MOVSD_LABEL_REG    "    movsd %s(%%rip), %s    # Load float immediate: %s\n"

// ========== DOUBLE PRECISION INSTRUCTIONS ==========
#define ASM_ADDSS               "addss"   // 32-bit float add
#define ASM_SUBSS               "subss"   // 32-bit float sub  
#define ASM_MULSS               "mulss"   // 32-bit float mul
#define ASM_DIVSS               "divss"   // 32-bit float div
#define ASM_MOVSS               "movss"   // 32-bit float move
#define ASM_UCOMISS             "ucomiss" // 32-bit float compare

// ========== DOUBLE OPERATION TEMPLATES ==========
#define ASM_TEMPLATE_MOVSD_MEM_REG_DOUBLE   "    movsd %d(%%rbp), %s    # Load double %s\n"
#define ASM_TEMPLATE_MOVSD_REG_MEM_DOUBLE   "    movsd %s, %d(%%rbp)    # Store double %s\n"
#define ASM_TEMPLATE_MOVSS_MEM_REG_FLOAT    "    movss %d(%%rbp), %s    # Load float %s\n"  
#define ASM_TEMPLATE_MOVSS_REG_MEM_FLOAT    "    movss %s, %d(%%rbp)    # Store float %s\n"

#define ASM_TEMPLATE_DOUBLE_CMP             "    ucomisd %s, %s\n"
#define ASM_TEMPLATE_FLOAT_CMP_SS           "    ucomiss %s, %s\n"

// String operations
#define ASM_TEMPLATE_LEAQ_LABEL_REG     "    leaq %s(%%rip), %s    # Load string: %s\n"

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

// Add these to your existing asmTemplate.h file

// ========== ADDITIONAL ASSEMBLY INSTRUCTIONS ==========
#define ASM_CLTD                "cltd"
#define ASM_CVTTSD2SI           "cvttsd2si"
#define ASM_CALL                "call"
#define ASM_RET                 "ret"

// ========== ADDITIONAL INSTRUCTION TEMPLATES ==========
// Division operations
#define ASM_TEMPLATE_DIV_EXTEND         "    cltd              # Sign extend EAX to EDX:EAX\n"
#define ASM_TEMPLATE_DIV_MOVE_TO_EAX    "    movl %s, %%eax\n"
#define ASM_TEMPLATE_DIV_MOVE_TO_ECX    "    movl %s, %%ecx\n"
#define ASM_TEMPLATE_IDIV               "    idivl %s\n"
#define ASM_TEMPLATE_IDIV_ECX           "    idivl %%ecx\n"
#define ASM_TEMPLATE_MOVE_EAX_TO_REG    "    movl %%eax, %s\n"
#define ASM_TEMPLATE_MOVE_EDX_TO_REG    "    movl %%edx, %s\n"

// Float conversion
#define ASM_TEMPLATE_FLOAT_TO_INT       "    cvttsd2si %s, %%rdi  # Convert float to int (simplified)\n"

// Function calls
#define ASM_TEMPLATE_CALL_RUNTIME       "    call %s     # %s\n"

// Register moves with comments
#define ASM_TEMPLATE_MOVQ_REG_REG   "    movq %s, %s       # %s\n"
#define ASM_TEMPLATE_MOVE_32_COMMENT    "    movl %s, %s       # %s\n"

// Struct member access
#define ASM_TEMPLATE_STORE_STRUCT_MEMBER        "    mov%s %s, %d(%%rbp)     # Store to struct member\n"
#define ASM_TEMPLATE_LOAD_STRUCT_MEMBER_FLOAT   "    movsd %d(%%rbp), %s    # Load struct member\n"
#define ASM_TEMPLATE_LOAD_STRUCT_MEMBER         "    mov%s %d(%%rbp), %s     # Load struct member\n"
#define ASM_TEMPLATE_STORE_BACK_STRUCT_FLOAT    "    movsd %s, %d(%%rbp)    # Store back to struct member\n"
#define ASM_TEMPLATE_STORE_BACK_STRUCT          "    mov%s %s, %d(%%rbp)     # Store back to struct member\n"

// Float operations with specific registers
#define ASM_TEMPLATE_MOVSS_REG_MEM      "    movss %s, %d(%%rbp)    # Store float %s\n"
#define ASM_TEMPLATE_MOVSS_MEM_REG      "    movss %d(%%rbp), %s    # Load float %s\n"

// Bool operations
#define ASM_TEMPLATE_MOVZBL_MEM_REG     "    movzbl %d(%%rbp), %s    # Load bool %s (zero-extended)\n"
#define ASM_TEMPLATE_MOVZBL_AL_REG      "    movzbl %%al, %s\n"
#define ASM_TEMPLATE_MOVL_IMM_REG       "    movl $%d, %s    # Load bool: %s\n"
#define ASM_TEMPLATE_MOVL_IMM_REG_INT   "    movl $%s, %s    # Load int: %s\n"

// Stack operations with comments
#define ASM_TEMPLATE_SUBQ_RSP   "    subq $%d, %%rsp    # Allocate %s (%d bytes)\n"

// Function parameter storage
#define ASM_TEMPLATE_STORE_PARAM        "    mov%s %s, %d(%%rbp)    # Store param %s\n"

// Spill/restore operations
#define ASM_TEMPLATE_SPILL_FLOAT        "    movsd %s, -%d(%%rbp)        # Spill float to tempVar\n"
#define ASM_TEMPLATE_SPILL_REG          "    mov%s %s, -%d(%%rbp)         # Spill to tempVar\n"
#define ASM_TEMPLATE_RESTORE_REG        "    mov%s -%d(%%rbp), %s           # Restore from tempVar\n"
#define ASM_TEMPLATE_RESTORE_FLOAT_MEM  "    movsd %d(%%rbp), %s    # Load %s\n"
#define ASM_TEMPLATE_ADDQ_RSP           "    addq $8, %s     # Deallocate stack space\n"

// Runtime function calls with specific formatting
#define ASM_RUNTIME_PRINT_STR_Z         "print_str_z"
#define ASM_RUNTIME_PRINT_INT           "print_int"
#define ASM_RUNTIME_PRINT_BOOL          "print_bool"
#define ASM_RUNTIME_EXIT_PROGRAM        "exit_program"

#define ASM_TEMPLATE_CALL_PRINT_STR     "    call print_str_z     # Runtime calculates length & prints\n"
#define ASM_TEMPLATE_CALL_PRINT_INT     "    call print_int       # Runtime converts & prints\n"
#define ASM_TEMPLATE_CALL_PRINT_BOOL    "    call print_bool      # Runtime prints 'true'/'false'\n"
#define ASM_TEMPLATE_CALL_EXIT          "    call exit_program    # Runtime exits cleanly\n"

// Struct allocation comments
#define ASM_TEMPLATE_STRUCT_ALLOC_COMMENT   "    # Allocate struct %s (size=%d)\n"

// Additional register specifications
#define ASM_REG_EAX             "%eax"
#define ASM_REG_EBX             "%ebx"
#define ASM_REG_ECX             "%ecx"
#define ASM_REG_EDX             "%edx"
#define ASM_REG_ESI             "%esi"
#define ASM_REG_EDI             "%edi"
#define ASM_REG_R8D             "%r8d"
#define ASM_REG_R9D             "%r9d"
#define ASM_REG_R10D            "%r10d"
#define ASM_REG_R11D            "%r11d"

// 8-bit registers
#define ASM_REG_BL              "%bl"
#define ASM_REG_CL              "%cl"
#define ASM_REG_DL              "%dl"
#define ASM_REG_SIL             "%sil"
#define ASM_REG_DIL             "%dil"
#define ASM_REG_R8B             "%r8b"
#define ASM_REG_R9B             "%r9b"
#define ASM_REG_R10B            "%r10b"
#define ASM_REG_R11B            "%r11b"

// ========== HELPER MACROS ==========
#define ASM_EMIT_CALL(file, function) \
    fprintf(file, ASM_TEMPLATE_CALL, function)

#define ASM_EMIT_CALL_RUNTIME(file, function, comment) \
    fprintf(file, ASM_TEMPLATE_CALL_RUNTIME, function, comment)

#define ASM_EMIT_STRUCT_ALLOC(file, name, size) \
    fprintf(file, ASM_TEMPLATE_STRUCT_ALLOC_COMMENT, name, size)

#define ASM_EMIT_SPILL_REG(file, suffix, reg, offset) \
    fprintf(file, ASM_TEMPLATE_SPILL_REG, suffix, reg, offset)

#define ASM_EMIT_RESTORE_REG(file, suffix, offset, reg) \
    fprintf(file, ASM_TEMPLATE_RESTORE_REG, suffix, offset, reg)

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