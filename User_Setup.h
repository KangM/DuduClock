// User_Setup.h for DuduClock - ST7789 240x240 on ESP32
// 此文件用于GitHub Actions编译，实际烧录请使用你硬件对应的引脚配置

#ifndef USER_SETUP_LOADED
#define USER_SETUP_LOADED

// 驱动芯片
#define ST7789_DRIVER
// #define TFT_SDA_READ  // 如果使用MISO读取，取消注释

// 屏幕分辨率
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// 颜色顺序
#define TFT_RGB_ORDER TFT_BGR

// SPI引脚 (常见DuduClock硬件接线)
#define TFT_MISO  19
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
// #define TFT_BL    15  // 背光（如果硬件连接了）

// SPI频率
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

// 字体内存优化
#define SMOOTH_FONT

#endif // USER_SETUP_LOADED
