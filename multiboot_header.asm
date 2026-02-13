; multiboot_header.asm
section .multiboot
align 4

; Multiboot 1 規範常數
MAGIC    equ 0x1BADB002
FLAGS    equ 0x00000004 | 0x00000002 ; Bit 2=Graphics, Bit 1=Memory Info
CHECKSUM equ -(MAGIC + FLAGS)

; Header 結構
dd MAGIC
dd FLAGS
dd CHECKSUM

; header_addr, load_addr, load_end_addr, bss_end_addr, entry_addr
; 設為 0 表示我們讓 Linker 決定，不手動指定
dd 0, 0, 0, 0, 0

; Graphics Mode Request (只有當 Bit 2 被設定時才有效)
dd 0          ; mode_type: 0 = linear graphics
dd 1024       ; width
dd 768        ; height
dd 32         ; depth (bits per pixel)
