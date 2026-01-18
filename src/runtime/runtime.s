# runtime functions x86_64 AT&T
.text
.globl _start
.globl print_str_z
.globl print_int
.globl print_float
.globl print_double
.globl print_bool
.globl print_newline
.globl read_int
.globl read_str
.globl exit_program

_start:
    call main
    movq $0, %rdi
    call exit_program

print_str_z:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rdi
    xorq %rcx, %rcx
strlen_loop:
    movb (%rdi, %rcx), %al
    testb %al, %al
    jz strlen_done
    incq %rcx
    jmp strlen_loop
strlen_done:
    movq $1, %rax
    movq $1, %rdi
    popq %rsi
    movq %rcx, %rdx
    syscall
    popq %rbp
    ret 

print_int:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rdi, %rax
    leaq 31(%rsp), %rdi
    movb $0, (%rdi)
    movq $10, %rcx
    testq %rax, %rax
    jns positive
    negq %rax
    pushq %rax
    movq $1, %r8
    popq %rax
    jmp convert
positive:
    xorq %r8, %r8
convert:
    decq %rdi
    xorq %rdx, %rdx
    divq %rcx
    addb $'0', %dl
    movb %dl, (%rdi)
    testq %rax, %rax
    jnz convert

    testq %r8, %r8
    jz print_number
    decq %rdi
    movb $'-', (%rdi)
print_number:
    call print_str_z
    addq $32, %rsp
    popq %rbp
    ret

print_float:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    
    # Convert single precision to double precision
    cvtss2sd %xmm0, %xmm0
    
    # Save original value
    movsd %xmm0, -8(%rbp)
    
    # Check if negative
    xorpd %xmm1, %xmm1
    ucomisd %xmm1, %xmm0
    jae float_positive
    
    # Print minus and make positive
    leaq minus_str(%rip), %rdi
    call print_str_z
    movsd -8(%rbp), %xmm0
    movsd neg_mask(%rip), %xmm1
    andpd %xmm1, %xmm0
    movsd %xmm0, -8(%rbp)
    
float_positive:
    # Print integer part
    movsd -8(%rbp), %xmm0
    cvttsd2si %xmm0, %rdi
    call print_int
    
    # Print decimal point
    leaq dot_str(%rip), %rdi
    call print_str_z
    
    # Calculate fractional part
    movsd -8(%rbp), %xmm0
    cvttsd2si %xmm0, %rax
    cvtsi2sd %rax, %xmm1
    subsd %xmm1, %xmm0
    
    # Multiply by 1000000 for 6 decimal places
    movsd float_scale(%rip), %xmm2
    mulsd %xmm2, %xmm0
    cvttsd2si %xmm0, %rdi
    
    # Print with leading zeros if needed
    subq $32, %rsp
    movq %rdi, %rax
    leaq 31(%rsp), %rdi
    movb $0, (%rdi)
    movq $10, %rcx
    movq $6, %r9
    
float_convert:
    decq %rdi
    xorq %rdx, %rdx
    divq %rcx
    addb $'0', %dl
    movb %dl, (%rdi)
    decq %r9
    testq %rax, %rax
    jnz float_convert
    
    # Add leading zeros
float_pad:
    testq %r9, %r9
    jz float_print
    decq %rdi
    movb $'0', (%rdi)
    decq %r9
    jmp float_pad
    
float_print:
    call print_str_z
    addq $32, %rsp
    addq $16, %rsp
    popq %rbp
    ret

print_double:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    
    # Save original value
    movsd %xmm0, -8(%rbp)
    
    # Check if negative
    xorpd %xmm1, %xmm1
    ucomisd %xmm1, %xmm0
    jae double_positive
    
    # Print minus and make positive
    leaq minus_str(%rip), %rdi
    call print_str_z
    movsd -8(%rbp), %xmm0
    movsd neg_mask(%rip), %xmm1
    andpd %xmm1, %xmm0
    movsd %xmm0, -8(%rbp)
    
double_positive:
    # Print integer part
    movsd -8(%rbp), %xmm0
    cvttsd2si %xmm0, %rdi
    call print_int
    
    # Print decimal point
    leaq dot_str(%rip), %rdi
    call print_str_z
    
    # Calculate fractional part
    movsd -8(%rbp), %xmm0
    cvttsd2si %xmm0, %rax
    cvtsi2sd %rax, %xmm1
    subsd %xmm1, %xmm0
    
    # Multiply by 1000000 for 6 decimal places
    movsd float_scale(%rip), %xmm2
    mulsd %xmm2, %xmm0
    cvttsd2si %xmm0, %rdi
    
    # Print with leading zeros if needed
    subq $32, %rsp
    movq %rdi, %rax
    leaq 31(%rsp), %rdi
    movb $0, (%rdi)
    movq $10, %rcx
    movq $6, %r9
    
double_convert:
    decq %rdi
    xorq %rdx, %rdx
    divq %rcx
    addb $'0', %dl
    movb %dl, (%rdi)
    decq %r9
    testq %rax, %rax
    jnz double_convert
    
    # Add leading zeros
double_pad:
    testq %r9, %r9
    jz double_print
    decq %rdi
    movb $'0', (%rdi)
    decq %r9
    jmp double_pad
    
double_print:
    call print_str_z
    addq $32, %rsp
    addq $16, %rsp
    popq %rbp
    ret

print_bool:
    pushq %rbp
    movq %rsp, %rbp
    
    testq %rdi, %rdi
    jz print_false
    
    leaq true_str(%rip), %rdi
    call print_str_z
    jmp bool_done
    
print_false:
    leaq false_str(%rip), %rdi
    call print_str_z
    
bool_done:
    popq %rbp
    ret

print_newline:
    movq $1, %rax
    movq $1, %rdi
    lea newline(%rip), %rsi
    movq $1, %rdx
    syscall
    ret


read_int:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    
    # Read from stdin (syscall 0)
    movq $0, %rax
    movq $0, %rdi
    leaq -32(%rbp), %rsi
    movq $32, %rdx
    syscall
    
    # Parse the input string to integer
    leaq -32(%rbp), %rsi
    xorq %rax, %rax
    xorq %rcx, %rcx
    
read_int_skip_ws:
    movb (%rsi), %dl
    cmpb $' ', %dl
    je read_int_ws_found
    cmpb $'\t', %dl
    je read_int_ws_found
    cmpb $'\n', %dl
    je read_int_ws_found
    cmpb $'\r', %dl
    je read_int_ws_found
    jmp read_int_check_sign
    
read_int_ws_found:
    incq %rsi
    jmp read_int_skip_ws
    
read_int_check_sign:
    movb (%rsi), %dl
    cmpb $'-', %dl
    jne read_int_parse
    movq $1, %rcx
    incq %rsi
    
read_int_parse:
    movb (%rsi), %dl
    cmpb $0, %dl
    je read_int_done
    cmpb $'\n', %dl
    je read_int_done
    cmpb $'\r', %dl
    je read_int_done
    cmpb $'0', %dl
    jb read_int_done
    cmpb $'9', %dl
    ja read_int_done
    
    imulq $10, %rax
    subb $'0', %dl
    movzbq %dl, %rdx
    addq %rdx, %rax
    
    incq %rsi
    jmp read_int_parse
    
read_int_done:
    testq %rcx, %rcx
    jz read_int_return
    negq %rax
    
read_int_return:
    addq $32, %rsp
    popq %rbp
    ret

read_str:
    pushq %rbp
    movq %rsp, %rbp
    
    # Read into global buffer (NO parameters needed!)
    movq $0, %rax
    movq $0, %rdi
    leaq string_buffer(%rip), %rsi    # Global buffer
    movq $255, %rdx
    syscall
    
    # Save bytes read
    movq %rax, %r10
    testq %r10, %r10
    jle read_str_empty
    
    # Null-terminate
    leaq string_buffer(%rip), %rdi
    addq %r10, %rdi
    movb $0, (%rdi)
    
    # Strip newline
    decq %rdi
    cmpb $'\n', (%rdi)
    jne read_str_check_cr
    movb $0, (%rdi)
    decq %rdi
    
read_str_check_cr:
    leaq string_buffer(%rip), %rsi
    cmpq %rsi, %rdi
    jl read_str_done
    cmpb $'\r', (%rdi)
    jne read_str_done
    movb $0, (%rdi)
    jmp read_str_done
    
read_str_empty:
    leaq string_buffer(%rip), %rdi
    movb $0, (%rdi)
    
read_str_done:
    # Return pointer in %rax
    leaq string_buffer(%rip), %rax
    popq %rbp
    ret

exit_program:
    movq $60, %rax
    syscall

.section .rodata
newline:
    .asciz "\n"
true_str:
    .asciz "true"
false_str:
    .asciz "false"
minus_str:
    .asciz "-"
dot_str:
    .asciz "."

.align 16
float_scale:
    .double 1000000.0
neg_mask:
    .quad 0x7FFFFFFFFFFFFFFF
    .quad 0x7FFFFFFFFFFFFFFF

.bss
.align 8
string_buffer:
    .space 256