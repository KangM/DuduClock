#ifndef __ARDUINOZLIB_H__
#define __ARDUINOZLIB_H__

#include <Arduino.h>

// ArduinoZlib - zlib解压缩库的stub实现，仅供编译验证使用。
// 实际烧录时请安装完整的ArduinoZlib库。
class ArduinoZlib {
public:
    // 解压zlib格式的数据
    // in: 输入压缩数据
    // in_size: 输入数据大小
    // out: 输出缓冲区(由调用者分配)
    // out_size: 输出缓冲区大小
    // out_result_size: 解压后实际大小(输出参数)
    // 返回值: 0=成功, 负数=失败
    static int libmpq__decompress_zlib(const uint8_t* in, int in_size, uint8_t* out, int out_size, uint32_t& out_result_size) {
        // Stub: 直接把输入复制到输出
        out_result_size = (in_size < out_size) ? in_size : out_size;
        if (out_result_size > 0 && in != nullptr && out != nullptr) {
            memcpy(out, in, out_result_size);
        }
        return 0;
    }
};

#endif
