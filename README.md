# DuduClock 2.2

基于 ESP32-C3 和 `TFT_eSPI` 的桌面天气时钟项目，支持实况天气、未来天气、空气质量、主题切换、计时器、网页配网和网页天气配置。

当前仓库已经整理为适合 `arduino-cli` 和 GitHub Actions 使用的结构，支持通过编译宏注入 `TFT_eSPI` 屏幕参数，不再依赖手工修改本机全局库。

## 功能概览

- 实况天气主页
  - 时间、日期、星期显示
  - 当前城市天气图标与天气文字
  - 温度、湿度显示
  - 城市空气质量等级标记
  - 底部轮播天气信息
- 空气质量页面
  - AQI
  - PM10
  - PM2.5
  - NO2
  - SO2
  - CO
  - O3
- 未来天气页面
  - 未来 6 日天气预报
  - 每日天气图标、天气文字、昼夜温度
- 主题切换页面
  - 支持黑色/白色主题
  - 当前默认主题为白色
- 计时器页面
  - 单击开始/停止
  - 长按归零
- 恢复设置页面
  - 长按恢复出厂设置
- 网页配置
  - 首次启动 AP 配网
  - 连接路由器后，通过局域网页面配置天气参数
  - WiFi 配置页和天气配置页均支持默认回填当前配置

## 版本说明

结合 [版本介绍.txt](./版本介绍.txt) 可整理为：

### 2.2（2025-05-25）

- 将和风天气空气质量接口从旧版本调整到 `V1`
- 背景原因是和风天气空气质量 `V7` 接口在 2026 年停止服务

### 2.1（2025-05-15）

- 适配和风天气新版身份认证
- 引入作者自写的 `DuduUtil` 库用于和风 JWT 生成

### 2.0（2024-03-25）

- 配网页增加“上级行政区划”输入，用于区分重名城市
- 从易客天气切换到和风天气
- 增加空气质量页面

## 当前仓库中已完成的工程化改动

- 将 WiFi 配置与天气配置拆成两个阶段
  - 没有 WiFi 信息时，进入 AP 配网页
  - 配网保存后自动重启
  - 设备连上局域网后，访问设备 IP 进入天气配置页
  - 天气配置保存后自动重启
- Web 配置页常驻
  - `http://设备IP/` 为天气配置页
  - `http://设备IP/wifi` 为 WiFi 配置页
  - `http://设备IP/city` 也可进入天气配置页
- 解决了湿度显示为 `0%` 的问题
  - 当前温度下方显示的是湿度，不是空气质量
- 将 `TFT_eSPI` 构建方式整理为“支持编译宏注入”
- 补充了 GitHub Actions workflow 和本地参数化构建脚本

## 硬件与接线

### 已验证的屏幕宏参数

当前仓库默认采用这组 `TFT_eSPI` 宏配置：

```c
#define ST7789_DRIVER
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_RGB_ORDER TFT_BGR
#define TFT_MISO -1
#define TFT_MOSI 3
#define TFT_SCLK 2
#define TFT_CS 7
#define TFT_DC 4
#define TFT_RST 5
```

说明：

- 当前项目实测以 `240x320` 为准
- 是否需要使用这些宏，取决于你本机 `TFT_eSPI` 是否已经手工改过
- 如果你恢复 `TFT_eSPI` 默认配置，就建议在编译时显式传入这些宏

### 网购转接板接线图

下面这张图是你提供的专用转接板接线参考：

![网购转接板接线](./网购转接板接线.jpg)

从图中可读出的常用定义是：

```c
//#define TFT_MISO 5
#define TFT_MOSI 3
#define TFT_SCLK 2
#define TFT_CS   7
#define TFT_DC   4
#define TFT_RST  5
//#define TFT_BL   13
```

### 按键

当前按键定义为：

- `BUTTON = GPIO9`

也就是复用了开发板上的 `BOOT` 键。

按键行为：

- 单击
  - 只在计时器页面有效
  - 开始/停止计时
- 双击
  - 在各个页面之间切换
- 长按 2 秒
  - 在主题页面切换主题
  - 在恢复设置页面恢复出厂设置

## 启动与配置流程

### 首次启动

1. 设备启动
2. 如果没有保存 WiFi 信息，进入 AP 模式
3. 手机或电脑连接设备 AP
4. 打开网页进行 WiFi 配置
5. 保存后设备自动重启

### 配网完成后

1. 设备连接到已配置的路由器
2. 屏幕显示：
   - 第一行：`天气配置`
   - 第二行：设备局域网 IP
3. 浏览器访问 `http://设备IP/`
4. 填写城市、上级行政区、和风天气密钥信息
5. 保存后设备自动重启

### 配置页访问路径

- `http://设备IP/`：天气配置页
- `http://设备IP/wifi`：WiFi 配置页
- `http://设备IP/city`：天气配置页

## 页面说明

### 天气主页

- 中间显示时间
- 顶部显示品牌标题
- 左上显示城市
- 城市右侧显示空气质量等级
- 中部显示天气图标
- 下部显示温度与湿度
- 底部轮播天气描述、体感温度、风向风力、能见度等信息

### 空气质量页

用于查看各污染物数据和空气质量等级。

### 未来天气页

显示未来 6 日天气预报。

### 主题页

长按切换黑白主题。

### 计时器页

- 单击开始
- 再次单击停止
- 长按归零

### 恢复设置页

长按恢复出厂设置。

## 和风天气配置项

当前需要在天气配置页填写：

- `city`
- `adm`
- `privateKey`
- `publicKey`
- `keyId`
- `projectId`
- `apiHost`

项目使用 `DuduUtil` 生成和风 JWT。

## 编译与烧录参数

### Arduino 菜单参数

根据 [烧录参数.png](./烧录参数.png)：

- 开发板：`AirM2M_CORE_ESP32C3`
- USB CDC On Boot：`Enabled`
- CPU Frequency：`80MHz (WiFi)`
- Core Debug Level：`None`
- Erase All Flash Before Sketch Upload：`Enabled`
- Flash Frequency：`80MHz`
- Partition Scheme：`Huge APP (3MB No OTA/1MB SPIFFS)`
- Upload Speed：`921600`

### arduino-cli FQBN

本仓库当前脚本与 workflow 使用：

```text
esp32:esp32:AirM2M_CORE_ESP32C3:UploadSpeed=921600,CDCOnBoot=default,CPUFreq=80,FlashFreq=80,PartitionScheme=huge_app,DebugLevel=none,EraseFlash=all
```

## 本地构建与烧录

### 脚本

本仓库提供：

- [build-flash-monitor.ps1](./build-flash-monitor.ps1)

支持：

- 编译
- 烧录
- 自动串口监视
- 使用默认库配置
- 使用 `TFT_eSPI` 编译宏注入

### 常用用法

使用默认库配置：

```powershell
.\build-flash-monitor.ps1 -p COM8
```

使用当前 240x320 ST7789 预设宏：

```powershell
.\build-flash-monitor.ps1 -p COM8 -TftPreset c3-240x320
```

手动自定义 TFT 参数：

```powershell
.\build-flash-monitor.ps1 -p COM8 -UseTftFlags -TftPreset custom -TftMosi 3 -TftSclk 2 -TftCs 7 -TftDc 4 -TftRst 5 -TftWidth 240 -TftHeight 320
```

只打开串口监视器：

```powershell
.\build-flash-monitor.ps1 -p COM8 -Monitor
```

## GitHub Actions

仓库包含：

- [.github/workflows/build.yml](./.github/workflows/build.yml)

支持：

- tag 推送时自动编译并生成 Release
- Release 里只提供一个合并后的可刷机 `.bin`

### 使用建议

- 如果你的本机 `TFT_eSPI` 已经手工改过并且稳定可用，可以先测试默认配置
- 如果你希望 workflow、不同机器、本地 `arduino-cli` 构建结果尽量一致，建议用编译宏注入
- 发版时请直接看 GitHub Release 正文里的刷机说明，里面会写明合并后的单文件 `.bin` 名称

## 目录说明

- `DuduClock.ino`：主入口与启动流程
- `net.cpp` / `net.h`：WiFi、网页配置、天气接口、NTP
- `task.cpp` / `task.h`：页面绘制与定时任务
- `tftUtil.cpp` / `tftUtil.h`：TFT 初始化与基础文字绘制
- `preferencesUtil.cpp` / `preferencesUtil.h`：NVS 配置读写
- `libraries/DuduUtil`：JWT 相关工具库
- `libraries/ArduinoZlib`：和风接口 gzip 解压
- `.github/workflows/build.yml`：CI 编译
- `build-flash-monitor.ps1`：本地参数化构建与烧录脚本

## 已知注意事项

- 当前源码中仍有一部分历史乱码注释，但不影响编译和运行
- `TFT_eSPI` 是否能亮屏，取决于你是否使用了正确的驱动、分辨率、颜色顺序和引脚
- 如果屏幕背光亮但没有图像，优先检查：
  - `MOSI/SCLK/CS/DC/RST`
  - `240x320`
  - `ST7789_DRIVER`
  - `TFT_RGB_ORDER`
- 如果屏幕中文显示 `□`，通常是字库中没有该汉字，不一定是程序逻辑问题

## 致谢

- 原始项目作者与版本说明提供者
- 和风天气
- `TFT_eSPI`
- `OneButton`
- `TaskScheduler`
