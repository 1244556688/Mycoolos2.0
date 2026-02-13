#include <stdint.h>

// 1. 定義 Multiboot 結構 (嚴格對齊)
typedef struct {
    uint32_t flags;
    uint32_t unused[11];
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
} __attribute__((packed)) multiboot_info_t;

// 2. 全域變數
uint32_t* vram;

// 3. 基礎字型庫 (為了穩定，我們先內建幾個必要的字元)
// 每個 byte 代表一行 (8 pixels)
unsigned char font_8x16[128][16] = {
    ['M'] = {0xC3, 0xE7, 0xFF, 0xFF, 0xDB, 0xC3, 0xC3, 0xC3, 0xC3},
    ['y'] = {0xC3, 0xC3, 0xC3, 0x7E, 0x3C, 0x18, 0x3C, 0x7E, 0x00},
    ['O'] = {0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00},
    ['S'] = {0x7C, 0xC6, 0xC0, 0x7C, 0x06, 0xC6, 0x7C, 0x00, 0x00},
    ['N'] = {0xC3, 0xE3, 0xF3, 0xDB, 0xCF, 0xC7, 0xC3, 0xC3, 0x00},
    ['o'] = {0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00},
    ['t'] = {0x10, 0x10, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x0E, 0x00},
    ['e'] = {0x00, 0x00, 0x7C, 0xC6, 0xFE, 0xC0, 0xC6, 0x7C, 0x00},
    ['p'] = {0x00, 0x00, 0x7C, 0xC6, 0xC6, 0x7C, 0xC0, 0xC0, 0xC0},
    ['a'] = {0x00, 0x00, 0x7C, 0x06, 0x7E, 0xC6, 0xC6, 0x7E, 0x00},
    ['d'] = {0x06, 0x06, 0x7E, 0xC6, 0xC6, 0xC6, 0xC6, 0x7E, 0x00},
    [':'] = {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00},
    ['>'] = {0xC0, 0x60, 0x30, 0x18, 0x30, 0x60, 0xC0, 0x00, 0x00}
};

// 4. 繪圖函式
void draw_rect(int x, int y, int w, int h, uint32_t color, uint32_t width) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            vram[i * width + j] = color;
        }
    }
}

void draw_char(int x, int y, char c, uint32_t color, uint32_t width) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if (font_8x16[(int)c][i] & (0x80 >> j)) {
                vram[(y + i) * width + (x + j)] = color;
            }
        }
    }
}

void draw_string(int x, int y, const char* str, uint32_t color, uint32_t width) {
    while (*str) {
        draw_char(x, y, *str++, color, width);
        x += 8;
    }
}

// 5. 主程式
void kmain(multiboot_info_t* mbi) {
    // 檢查是否有圖形模式旗標 (第 12 位)
    if (!(mbi->flags & (1 << 12))) return;

    vram = (uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    uint32_t w = mbi->framebuffer_width;
    uint32_t h = mbi->framebuffer_height;

    // A. 畫背景 (深青色)
    draw_rect(0, 0, w, h, 0x008080, w);

    // B. 畫視窗 (外框)
    int win_x = 100, win_y = 100, win_w = 400, win_h = 250;
    draw_rect(win_x, win_y, win_w, win_h, 0xC0C0C0, w); // 灰色面板
    draw_rect(win_x, win_y, win_w, 25, 0x000080, w);    // 藍色標題列
    draw_rect(win_x + 5, win_y + 30, win_w - 10, win_h - 35, 0xFFFFFF, w); // 白色內框

    // C. 印字
    draw_string(win_x + 5, win_y + 5, "MyOS Notepad", 0xFFFFFF, w);
    draw_string(win_x + 15, win_y + 40, "Welcome to MyOS!", 0x000000, w);
    draw_string(win_x + 15, win_y + 60, "> System Ready", 0x000000, w);

    while (1) {
        __asm__("hlt");
    }
}
