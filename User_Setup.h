// User_Setup.h for DuduClock - ST7789 240x240 on ESP32-C3
// 此文件用于GitHub Actions编译，请根据实际硬件接线修改引脚！

#ifndef USER_SETUP_LOADED
#define USER_SETUP_LOADED

// 驱动芯片
#define ST7789_DRIVER

// 屏幕分辨率
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// 颜色顺序
#define TFT_RGB_ORDER TFT_BGR

// SPI引脚（ESP32-C3 默认SPI：MOSI=7, SCLK=6, MISO=2）
// 请对照你的硬件接线修改 CS / DC / RST 引脚！
#define TFT_MISO  2
#define TFT_MOSI  7
#define TFT_SCLK  6
#define TFT_CS    10
#define TFT_DC    8
#define TFT_RST   4
// #define TFT_BL    5  // 背光（如果硬件连接了）

// SPI频率
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

// 字体内存优化
#define SMOOTH_FONT

#endif // USER_SETUP_LOADED
