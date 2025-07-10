; ============= INPUT and OUTPUT functions for user ============= //

section .data
    output_buffer times 16 db 0

section .text
    global in_syscall
    global out_syscall
    global hlt_syscall

; [in_syscall]: reads data from standard input
; no arguments
in_syscall:
    mov rax, 0                  ; syscall number for read
    mov rdi, 0                  ; stdin
    mov rsi, buffer             ; buffer to store input
    mov rdx, 16                 ; number of bytes to read
    syscall                     ;

    mov rsi, buffer             ;
    xor rax, rax                ; rax for result
    xor rbx, rbx                ; rbx for temp storage

.convert_loop:                  ; convert ASCII digits to number
    mov bl, [rsi]               ; next byte from buffer
    cmp bl, 0                   ; check for null terminator
    je .done                    ;
    cmp bl, 10                  ; check for newline
    je .done                    ;
    sub bl, '0'                 ; convert ASCII digit to numeric value
    cmp bl, 9                   ; check if valid digit (0-9)
    ja .error                   ;
    imul rax, 10                ; mul current result by 10
    add rax, rbx                ; add new digit to result
    inc rsi                     ; move to next byte
    jmp .convert_loop           ; loop

.done:
    ret

.error:
    mov rax, -1
    ret

; [out_syscall]: outputs the value of a register to console
; argument:      rdi - value to print (passed from caller)
out_syscall:
    mov rax, rdi                ; input value to rax
    mov rdi, output_buffer + 15 ; end of output buffer
    mov byte [rdi], 10          ; newline
    dec rdi                     ; buffer pointer back
    mov rcx, 0                  ; digit counter
    test rax, rax               ; check zero
    jnz .convert
    mov byte [rdi], '0'
    inc rcx
    jmp .write

.convert:                       ; number to ASCII
    mov rbx, 10                 ; divisor for decimal

.convert_loop:
    xor rdx, rdx
    div rbx
    add dl, '0'
    mov [rdi], dl
    dec rdi
    inc rcx
    test rax, rax
    jnz .convert_loop

.write:                         ; write buffer to stdout
    inc rdi
    mov rax, 1                  ; syscall number for write
    mov rsi, rdi                ; buffer address
    mov rdi, 1                  ; stdout
    mov rdx, rcx
    inc rdx
    syscall
    ret

; [hlt_syscall]: end of program
; no arguments
hlt_syscall:
    mov rax, 60
    mov rdi, 0
    syscall
    ret                         ; not reached
