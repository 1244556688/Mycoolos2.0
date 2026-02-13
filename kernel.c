#include <stdint.h>

/* --- 1. Multiboot 結構與定義 --- */
typedef struct {
    uint32_t flags;
    uint32_t unused[11];
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
} __attribute__((packed)) multiboot_info_t;

/* --- 2. 全域變數 --- */
uint32_t* vram;
extern uint8_t _backbuffer_start; // 引用 linker.ld 定義的空間
uint32_t* backbuffer;
uint32_t screen_w, screen_h;

/* 滑鼠與記事本狀態 */
int mouse_x = 200, mouse_y = 200;
char* notepad_title = "MyCoolOS Notepad v2.0";
char* notepad_content = "Hello! Your OS is alive.";

/* --- 3. 精簡 8x16 點陣字型庫 (示範用) --- */
// 這裡定義一個簡單的 8x16 字符點陣，1 代表有色，0 代表透明
unsigned char font_simple[128][16] = {
    ['H'] = {0x82,0x82,0x82,0x82,0xFE,0x82,0x82,0x82,0x82},
    ['e'] = {0x00,0x00,0x00,0x7C,0xC2,0xFE,0xC0,0x7E,0x00},
    ['l'] = {0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00},
    ['o'] = {0x00,0x00,0x00,0x7C,0xC2,0xC2,0xC2,0x7C,0x00},
    ['!'] = {0x30,0x30,0x30,0x30,0x30,0x00,0x30,0x30,0x00},
    ['M'] = {0xC3,0xE7,0xFF,0xDB,0xC3,0xC3,0xC3,0xC3,0x00},
    ['y'] = {0xC3,0xC3,0xC3,0x7E,0x3C,0x18,0x3C,0x7E,0x00},
    ['O'] = {0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00},
    ['S'] = {0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00,0x00}
};

/* --- 4. 繪圖核心函式 --- */

void put_pixel_back(int x, int y, uint32_t color) {
    if (x >= 0 && x < screen_w && y >= 0 && y < screen_h) {
        backbuffer[y * screen_w + x] = color;
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            put_pixel_back(x + j, y + i, color);
        }
    }
}

void draw_char(int x, int y, char c, uint32_t color) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if (font_simple[(int)c][i] & (0x80 >> j)) {
                put_pixel_back(x + j, y + i, color);
            }
        }
    }
}

void draw_string(int x, int y, const char* s, uint32_t color) {
    while (*s) {
        draw_char(x, y, *s++, color);
        x += 8;
    }
}

/* 繪製滑鼠游標 (箭頭造型) */
void draw_mouse(int x, int y) {
    uint32_t c = 0xFFFFFF; // 白色游標
    draw_rect(x, y, 2, 12, c);
    draw_rect(x, y, 8, 2, c);
    put_pixel_back(x+2, y+2, c);
    put_pixel_back(x+3, y+3, c);
    put_pixel_back(x+4, y+4, c);
}

/* --- 5. 核心邏輯 --- */

void kmain(multiboot_info_t* mbi) {
    vram = (uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    screen_w = mbi->framebuffer_width;
    screen_h = mbi->framebuffer_height;
    backbuffer = (uint32_t*)&_backbuffer_start;

    int frame = 0;
    while (1) {
        // 1. 繪製桌面背景
        draw_rect(0, 0, screen_w, screen_h, 0x008080); // 深青色

        // 2. 繪製記事本視窗
        int win_x = 150, win_y = 100, win_w = 400, win_h = 250;
        draw_rect(win_x, win_y, win_w, win_h, 0xCCCCCC);       // 邊框
        draw_rect(win_x + 2, win_y + 2, win_w - 4, 25, 0x000080); // 標題列
        draw_rect(win_x + 2, win_y + 30, win_w - 4, win_h - 32, 0xFFFFFF); // 內容區
        
        draw_string(win_x + 10, win_y + 6, "MyOS Notepad", 0xFFFFFF);
        draw_string(win_x + 20, win_y + 50, "Hello! MyOS", 0x000000);

        // 3. 處理滑鼠移動 (這裡先做一個自動圓周運動示範，待你加入中斷後改為真實座標)
        frame++;
        mouse_x = 300 + (frame % 200); 
        draw_mouse(mouse_x, mouse_y);

        // 4. 將 Backbuffer 一次推送到 VRAM (避免閃爍)
        for (uint32_t i = 0; i < screen_w * screen_h; i++) {
            vram[i] = backbuffer[i];
        }

        // 稍微延遲
        for (volatile int i = 0; i < 500000; i++);
    }
}
