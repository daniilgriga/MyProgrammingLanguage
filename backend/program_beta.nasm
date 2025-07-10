%include "src/io_syscalls.nasm"

section .data
    buffer times 256 db 0
    argc       dq 0
    numb       dq 0
    result     dq 0
    old        dq 0
               dq 0

section .text

global _start

carti:
    push rbp
    mov rbp, rsp
    mov rcx, 1
    mov [numb], rcx
    mov rdx, 1
    mov [result], rdx
    mov rsi, 1
    mov [old], rsi
    call in_syscall
    mov rcx, rax

loop_body383:
    cmp rcx, 0
    jle end_loop_body383
    imul rdx, rcx
    mov [result], rdx
    mov rdi, 1
    sub rcx, rdi
    mov [numb], rcx
    jmp loop_body383

end_loop_body383:
    mov rdi, rdx
    call out_syscall
    mov rsp, rbp
    pop rbp
    ret

_start:
    call carti
    call hlt_syscall
