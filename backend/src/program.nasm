section .data
    numb dq 0
    result dq 0
    old dq 0

section .text
global _start

carti:
    push rbp
    mov rbp, rsp
    mov rbx, 5
    mov [numb], rbx
    mov rcx, 1
    mov [result], rcx
    mov rdx, 1
    mov [old], rdx
loop_body383:
    cmp rbx, 0
    jle end_loop_body383
    mov [old], rbx
    mov [result], rcx
    mov rsi, 1
    mov [old], rbx
    mov rdi, 1
    mov [numb], rbx
    jmp loop_body383

end_loop_body383:
    mov rsp, rbp
    pop rbp
    ret

_start:
    call carti
    mov rax, 60
    xor rdi, rdi
    syscall
