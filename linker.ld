ENTRY(start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 1M;

    .text BLOCK(4K) : ALIGN(4K) {
        KEEP(*(.multiboot))
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
        /* 在這裡定義一個符號代表 BSS 結尾，也就是緩衝區起點 */
        _backbuffer_start = .;
        . += 0x400000; /* 預留 4MB 給 1024x768 畫面 */
    }
}
