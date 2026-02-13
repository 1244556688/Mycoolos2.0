; boot.asm
global start
extern kmain            ; 呼叫 C 語言的入口函數

section .text
bits 32

start:
    ; 1. 設定堆疊指標 (Stack Pointer)
    mov esp, stack_top

    ; 2. 處理 Multiboot 資訊
    ; GRUB 會把 Multiboot Info 的記憶體位址放在 EBX 暫存器
    push ebx 

    ; 3. 跳轉到 C 語言核心
    call kmain

    ; 4. 如果核心意外返回，讓 CPU 進入無窮迴圈
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB Kernel Stack
stack_top:
