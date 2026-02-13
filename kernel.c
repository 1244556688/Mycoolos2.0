#include <stdint.h>

/* --- 正確引用 Linker 中的符號 --- */
extern uint8_t _backbuffer_start; // 聲明為一個外部符號
uint32_t* backbuffer;
uint32_t* vram;

/* ... multiboot_info_t 定義保持不變 ... */

void kmain(multiboot_info_t* mbi) {
    if (!(mbi->flags & (1 << 12))) return;

    vram = (uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    uint32_t width = mbi->framebuffer_width;
    uint32_t height = mbi->framebuffer_height;

    /* 將 backbuffer 指向 Linker 預留的位置 */
    backbuffer = (uint32_t*)&_backbuffer_start; 

    /* 第一階段：先確認 VRAM 繪圖正常（紅色畫面） */
    for (uint32_t i = 0; i < width * height; i++) {
        vram[i] = 0xFF0000; 
    }

    /* 第二階段：確認 Backbuffer 繪圖正常（藍色小方塊） */
    /* 如果這段成功，紅色畫面上會出現藍色方塊 */
    for (int y = 100; y < 300; y++) {
        for (int x = 100; x < 300; x++) {
            backbuffer[y * width + x] = 0x0000FF;
        }
    }

    /* 將 backbuffer 內容同步到螢幕 */
    for (uint32_t i = 0; i < width * height; i++) {
        // 如果 backbuffer 有畫東西，就蓋掉紅色的 VRAM
        if (backbuffer[i] != 0) vram[i] = backbuffer[i];
    }

    while (1) {
        asm("hlt");
    }
}
