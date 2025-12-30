    .section .rodata
    .text
.LC0:
    .string "\n"


    .globl main
    .type main, @function
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $256, %rsp
    movl $29, %eax
    movl %eax, -4(%rbp)
    movl $45, %eax
    movl %eax, -8(%rbp)
    movl $10, %eax
    movl %eax, -12(%rbp)
    movl -12(%rbp), %eax
    movl -4(%rbp), %ecx
    addl %ecx, %eax
    movl %eax, -16(%rbp)
    movl -16(%rbp), %eax
    movl %eax, -12(%rbp)
    movl -12(%rbp), %edi
    call print_int
    leaq .LC0(%rip), %rdi
    call print_str_z
    movl $0, %eax
    movq %rbp, %rsp
    popq %rbp
    ret
