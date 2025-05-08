section .data
    bro dq 0
    dude dq 0
    yamal dq 0

section .text
global _start

carti:
    push rbp
    mov rbp, rsp
    mov rcx, 25
    mov [bro], rcx
    mov rdx, 10
    mov [dude], rdx
    push rdx
    call eval
    add rsp, 8
    mov rsi, r8
    mov [bro], rsi
    mov rsp, rbp
    pop rbp
    ret

eval:
    push rbp
    mov rbp, rsp
    mov r8, 9
    mov [yamal], r8
    mov rsp, rbp
    pop rbp
    ret

_start:
    call carti
    mov rax, 60
    xor rdi, rdi
    syscall
