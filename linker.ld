ENTRY(start)

SECTIONS {
    . = 1M;

    .text : {
        /* 確保 Multiboot Header 放在檔案的最前面 */
        KEEP(*(.multiboot))
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}
