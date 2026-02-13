; boot.asm 修正版
global start
extern kmain
global load_gdt

section .text
bits 32

start:
    mov esp, stack_top
    push ebx 
    call kmain
    cli
.hang: hlt
    jmp .hang

; 載入 GDT 的函式
load_gdt:
    mov eax, [esp + 4]
    lgdt [eax]
    ; 刷新段暫存器
    jmp 0x08:.flush
.flush:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

section .bss
align 16
stack_bottom: resb 16384
stack_top:
