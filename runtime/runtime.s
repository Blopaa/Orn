# runtime functions x86_64 AT&T
.text
.globl _start
.globl print_str_z
.globl print_int
.globl print_bool
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
    # this is for negatives
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
    abbd $'0', %dl
    movb %dl, (%rdi)
    testq %rax, %rax
    jnz convert

    testq %r8, %r8
    jz print_number
    decq %rdi
    movb  $'-', (%rdi)
print_number:
    call print_str_z
    addq $32, %rsp
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

exit_program:
    movq $60, %rax
    syscall

.section .rodata
true_str:
    .asciz "true"
false_str:
    .asciz "false"