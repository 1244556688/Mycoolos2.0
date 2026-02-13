#include <stdint.h>

/* --- 連接埠操作 (匯編包裝) --- */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/* --- 全域系統狀態 --- */
uint32_t *vram;          // 真正的螢幕位址
uint32_t *backbuffer;    // 後台緩衝區 (Double Buffer)
int screen_w, screen_h;

/* 滑鼠狀態 */
int mouse_x = 512, mouse_y = 384;
uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];

/* --- 雙重緩衝：畫面同步 --- */
void screen_update() {
    uint32_t size = screen_w * screen_h;
    for (uint32_t i = 0; i < size; i++) {
        vram[i] = backbuffer[i];
    }
}

/* --- 精美滑鼠游標 (16x16 點陣) --- */
void draw_cursor(int x, int y) {
    // 畫一個簡單的白色箭頭，帶黑色邊框
    for(int i=0; i<15; i++) {
        for(int j=0; j<i; j++) {
            // 這裡直接操作 backbuffer，在 update 之前畫上去
            backbuffer[(y+i) * screen_w + (x+j)] = 0xFFFFFF; 
        }
    }
}

/* --- PS/2 滑鼠驅動初始化 --- */
void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) if ((inb(0x64) & 1) == 1) return;
    } else {
        while (timeout--) if ((inb(0x64) & 2) == 0) return;
    }
}

void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
}

void init_mouse() {
    uint8_t status;
    mouse_wait(1);
    outb(0x64, 0xA8); // 開啟滑鼠埠
    mouse_wait(1);
    outb(0x64, 0x20); // 讀取 Comand Byte
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60); // 寫入 Comand Byte
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xF4); // 開啟數據傳輸
}

/* 處理滑鼠數據封包 */
void mouse_handler() {
    uint8_t status = inb(0x64);
    if (!(status & 1) || !(status & 0x20)) return; // 確保數據來自滑鼠

    mouse_byte[mouse_cycle++] = inb(0x60);
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        // 解析位移
        mouse_x += mouse_byte[1];
        mouse_y -= mouse_byte[2]; // Y 軸通常是反的
        
        // 邊界檢查
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x >= screen_w) mouse_x = screen_w - 1;
        if (mouse_y >= screen_h) mouse_y = screen_h - 1;
        
        // 檢查左鍵點擊 (mouse_byte[0] & 1)
    }
}

/* --- 記事本功能邏輯 (Notepad Core) --- */
char notepad_text[2048];
int text_ptr = 0;

void draw_notepad() {
    // 繪製背景視窗
    draw_rect_back(150, 100, 600, 400, 0xEEEEEE); // 視窗底色
    draw_rect_back(150, 100, 600, 30, 0x005A9E);  // 標題列 (Windows 10 藍)
    
    // 繪製文字區域
    draw_rect_back(160, 140, 580, 350, 0xFFFFFF); // 白色輸入區
    
    // 渲染文字 (這裡需要之前的 draw_string 配合)
    // render_text(170, 150, notepad_text, 0x000000);
}

/* --- 核心主循環優化 --- */
void kmain(multiboot_info_t *mb_info) {
    // ... 初始化 VRAM 與 記憶體分配 backbuffer (需實作 kmalloc) ...
    vram = (uint32_t*)mb_info->framebuffer_addr;
    screen_w = mb_info->framebuffer_width;
    screen_h = mb_info->framebuffer_height;
    
    // 簡單模擬分配：將 backbuffer 放在 8MB 處
    backbuffer = (uint32_t*)0x800000; 

    init_mouse();
    
    while(1) {
        // 1. 繪製背景與 GUI 元件 到 backbuffer
        draw_background(); 
        draw_notepad();
        
        // 2. 處理滑鼠數據 (實務上應由中斷驅動，這裡先輪詢示範)
        mouse_handler();
        
        // 3. 在所有 GUI 之上畫滑鼠
        draw_cursor(mouse_x, mouse_y);
        
        // 4. 將完成的 backbuffer 整塊推送到 VRAM
        screen_update();
        
        // 避免 CPU 空轉過熱
        asm("hlt");
    }
}
