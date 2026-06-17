// User_Setup.h for DuduClock
// 硬件：合宙 ESP32-C3-CORE + ST7789 240x240
// 引脚来源：https://wiki.luatos.com/chips/esp32c3/board.html

#ifndef USER_SETUP_LOADED
#define USER_SETUP_LOADED

#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define TFT_RGB_ORDER TFT_BGR

// SPI2 引脚（按合宙官方引脚表）
#define TFT_MISO  10   // SPI2_MISO
#define TFT_MOSI  3    // SPI2_MOSI
#define TFT_SCLK  2    // SPI2_CK
#define TFT_CS    7    // SPI2_CS

// DC / RST 用空闲 GPIO（4/5/6/1 都可以）
#define TFT_DC    6
#define TFT_RST   4
// #define TFT_BL  5    // 背光，按需取消注释

#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

#define SMOOTH_FONT

#endif
