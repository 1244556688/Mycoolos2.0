#include <stdint.h>

/* --- 1. Multiboot 結構定義 (必須放在最上方) --- */
typedef struct {
    uint32_t flags;
    uint32_t unused[11];
    /* Framebuffer info */
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
} __attribute__((packed)) multiboot_info_t;

/* --- 2. 全域變數與硬體抽象 --- */
uint32_t *vram;
uint32_t *backbuffer;
uint32_t screen_w, screen_h, screen_pitch;

int mouse_x = 100, mouse_y = 100;
char notepad_buffer[1024] = "Hello MyOS! Type here...";

/* 底層 I/O 函式 */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* --- 3. 繪圖核心 (優化版) --- */

// 畫矩形到後台緩衝區 (避免閃爍)
void draw_rect_back(int x, int y, int w, int h, uint32_t color) {
    if (x < 0 || y < 0 || x + w > screen_w || y + h > screen_h) return;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            backbuffer[(y + i) * screen_w + (x + j)] = color;
        }
    }
}

// 快速同步：將 Backbuffer 內容推送到 VRAM
void swap_buffers() {
    uint32_t total_pixels = screen_w * screen_h;
    for (uint32_t i = 0; i < total_pixels; i++) {
        vram[i] = backbuffer[i];
    }
}

/* --- 4. GUI 元件繪製 --- */

void draw_cursor(int x, int y) {
    // 畫一個簡單的紅色 8x8 方塊當下游標
    draw_rect_back(x, y, 8, 8, 0xFF0000);
}

void draw_notepad() {
    // 視窗主體
    draw_rect_back(150, 100, 500, 350, 0xCCCCCC); // 灰色邊框
    draw_rect_back(152, 102, 496, 28, 0x000080);  // 藍色標題列
    draw_rect_back(155, 135, 490, 310, 0xFFFFFF); // 白色編輯區
    
    // 這裡預留文字渲染空間
}

void draw_background() {
    // 深青色桌面
    draw_rect_back(0, 0, screen_w, screen_h, 0x008080);
    // 底部工作列
    draw_rect_back(0, screen_h - 40, screen_w, 40, 0x333333);
}

/* --- 5. 核心入口 --- */

void kmain(multiboot_info_t *mb_info) {
    // 1. 初始化顯示資訊
    vram = (uint32_t *)(uintptr_t)mb_info->framebuffer_addr;
    screen_w = mb_info->framebuffer_width;
    screen_h = mb_info->framebuffer_height;
    screen_pitch = mb_info->framebuffer_pitch;

    // 2. 設定後台緩衝區 (開發初期暫用 8MB 處的記憶體，實務需 Memory Manager)
    backbuffer = (uint32_t *)0x800000;

    // 3. 簡單的事件循環
    while (1) {
        // A. 繪製
        draw_background();
        draw_notepad();
        draw_cursor(mouse_x, mouse_y);

        // B. 同步到螢幕 (Double Buffering)
        swap_buffers();

        // C. 模擬滑鼠移動 (實際應由中斷處理，這裡先讓它慢慢動)
        mouse_x = (mouse_x + 1) % screen_w;
        
        // 稍微延遲 (防止 QEMU 跑太快看不清楚)
        for(volatile int i = 0; i < 1000000; i++);
    }
}
