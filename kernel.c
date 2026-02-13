/* kernel.c - MyOS GUI Kernel */
#include <stdint.h>

/* --- 1. 定義 Multiboot 結構 (簡化版) --- */
/* 我們需要讀取 GRUB 傳給我們的 VRAM 位址 */
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
    
    /* Framebuffer info (Multiboot 1 extension) */
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
} multiboot_info_t;

/* 全域變數：儲存螢幕資訊 */
uint32_t *fb_buffer;
uint32_t fb_pitch;
uint32_t fb_width;
uint32_t fb_height;
uint8_t  fb_bpp;

/* --- 2. 繪圖驅動層 (Graphics Driver) --- */

// 設定單一像素顏色 (格式: 0x00RRGGBB)
void put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= fb_width || y < 0 || y >= fb_height) return;
    
    // 計算記憶體偏移量： y * (每行位元組數 / 4) + x
    // 我們除以 4 是因為 uint32_t 是 4 bytes，而 pitch 是 byte 單位
    uint32_t offset = y * (fb_pitch / 4) + x;
    fb_buffer[offset] = color;
}

// 畫矩形 (實心)
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            put_pixel(x + i, y + j, color);
        }
    }
}

// 畫邊框 (空心)
void draw_border(int x, int y, int w, int h, uint32_t color, int thickness) {
    draw_rect(x, y, w, thickness, color);            // 上
    draw_rect(x, y + h - thickness, w, thickness, color); // 下
    draw_rect(x, y, thickness, h, color);            // 左
    draw_rect(x + w - thickness, y, thickness, h, color); // 右
}

/* --- 3. GUI 系統層 (Window System) --- */

void draw_desktop_background() {
    // 繪製一個漂亮的漸層背景 (藍色系)
    for (int y = 0; y < fb_height; y++) {
        for (int x = 0; x < fb_width; x++) {
            // 簡單的漸層演算法
            uint8_t r = 0;
            uint8_t g = (x * 100) / fb_width; 
            uint8_t b = 100 + (y * 155) / fb_height;
            uint32_t color = (r << 16) | (g << 8) | b;
            put_pixel(x, y, color);
        }
    }
}

void draw_taskbar() {
    // 底部灰色工作列
    draw_rect(0, fb_height - 40, fb_width, 40, 0xC0C0C0);
    // 工作列頂部亮線
    draw_rect(0, fb_height - 40, fb_width, 2, 0xFFFFFF);
    // 開始按鈕 (模擬)
    draw_rect(5, fb_height - 35, 60, 30, 0x008080); // 青色按鈕
}

// 模擬繪製一個視窗 (例如：記事本)
void draw_window(int x, int y, int w, int h, char* title) {
    // 1. 視窗陰影 (增加立體感)
    draw_rect(x + 5, y + 5, w, h, 0x202020);

    // 2. 視窗本體 (灰色背景)
    draw_rect(x, y, w, h, 0xE0E0E0);

    // 3. 標題列 (深藍色)
    draw_rect(x + 2, y + 2, w - 4, 25, 0x000080);

    // 4. 內容區域 (白色 - 記事本編輯區)
    draw_rect(x + 5, y + 30, w - 10, h - 35, 0xFFFFFF);

    // 5. 視窗邊框 (3D 效果)
    draw_border(x, y, w, h, 0xFFFFFF, 1); // 外亮邊
    draw_border(x+1, y+1, w-2, h-2, 0x808080, 1); // 內暗邊
}

/* --- 4. 核心入口 (Kernel Entry) --- */

void kmain(multiboot_info_t *mb_info) {
    // 檢查 Multiboot 標誌，確認是否有 Framebuffer 資訊
    if (mb_info->flags & (1 << 12)) {
        fb_buffer = (uint32_t *)(uintptr_t)mb_info->framebuffer_addr;
        fb_pitch = mb_info->framebuffer_pitch;
        fb_width = mb_info->framebuffer_width;
        fb_height = mb_info->framebuffer_height;
        fb_bpp = mb_info->framebuffer_bpp;
    } else {
        // 如果沒有圖形模式，這裡什麼都做不了 (或用 VGA 文字模式報錯)
        return; 
    }

    // --- GUI 初始化流程 ---
    
    // 1. 繪製桌面
    draw_desktop_background();

    // 2. 繪製工作列
    draw_taskbar();

    // 3. 繪製 MyOS Notepad 視窗
    //    位置 (100, 100)，大小 400x300
    draw_window(100, 100, 400, 300, "Notepad");

    // 4. 繪製一個模擬的滑鼠游標 (簡單的紅色箭頭)
    int mx = fb_width / 2;
    int my = fb_height / 2;
    draw_rect(mx, my, 10, 10, 0xFF0000); // 暫時用方塊代表滑鼠

    // 進入無窮迴圈，防止核心退出
    while(1) {
        // 未來這裡會放入 Event Loop
        // check_keyboard();
        // check_mouse();
    }
}
