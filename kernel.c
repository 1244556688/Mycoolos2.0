#include <stdint.h>

/* --- 1. 必須先定義結構體 --- */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr; // 這是我們畫圖需要的
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
} __attribute__((packed)) multiboot_info_t;

/* --- 2. 接著定義全域變數 --- */
uint32_t* vram;

/* --- 3. 最後才是函式實作 --- */
void kmain(multiboot_info_t* mbi) {
    // 檢查有沒有圖形資訊
    if (!(mbi->flags & (1 << 12))) return;

    vram = (uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    uint32_t width = mbi->framebuffer_width;
    uint32_t height = mbi->framebuffer_height;

    // 先測試：畫一個全紅螢幕
    for (uint32_t i = 0; i < width * height; i++) {
        vram[i] = 0xFF0000; 
    }

    // 在中間畫一個藍色小方塊 (記事本的預兆)
    for (uint32_t y = 100; y < 300; y++) {
        for (uint32_t x = 100; x < 300; x++) {
            vram[y * width + x] = 0x0000FF;
        }
    }

    while (1) {
        asm("hlt");
    }
}
