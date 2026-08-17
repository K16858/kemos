; System V AMD64 ABI helpers for freestanding kernel

bits 64
section .text

global LoadIDT
global IntHandlerInt40
extern IntHandlerInt40_C

; void LoadIDT(uint16_t limit, void* offset);
; rdi = limit, rsi = offset
LoadIDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di
    mov [rsp + 2], rsi
    lidt [rsp]
    add rsp, 10
    pop rbp
    ret

; Software interrupt stub for vector 0x40 (no error code)
align 16
IntHandlerInt40:
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    call IntHandlerInt40_C

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    iretq
