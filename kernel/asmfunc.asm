; System V AMD64 ABI helpers for freestanding kernel

bits 64
section .text

global LoadIDT
global IntHandlerInt40
global IntHandlerKeyboard
extern IntHandlerInt40_C
extern IntHandlerKeyboard_C

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

align 16
IntHandlerKeyboard:
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

    call IntHandlerKeyboard_C

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

; void IoOut8(uint16_t addr, uint8_t data);
; rdi = addr, rsi = data
global IoOut8
IoOut8:
    mov dx, di
    mov al, sil
    out dx, al
    ret

; uint8_t IoIn8(uint16_t addr);
; rdi = addr, return al
global IoIn8
IoIn8:
    mov dx, di
    in al, dx
    ret
