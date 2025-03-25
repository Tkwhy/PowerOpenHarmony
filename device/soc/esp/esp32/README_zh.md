# ESP32芯片平台

本目录存放ESP32芯片平台组件代码，代码基于[乐鑫官方SDK](https://github.com/espressif/esp-idf/tree/release/v4.3/components)删减、修改、适配而成。

## 目录结构

esp32目录结构如下所示：

```
.
├── BUILD.gn
├── Kconfig.liteos_m.defconfig.esp32_u4wdh	---	Kconfig配置
├── Kconfig.liteos_m.defconfig.series		
├── Kconfig.liteos_m.series						
├── Kconfig.liteos_m.soc						
├── README_zh.md							---	说明文档
└── components								---	组件目录
    ├── BUILD.gn
    ├── sdkconfig.gni						---	组件配置BUILD.gn头目录
    ├── sdkconfig.h							---	组件配置头目录
    ├── libs								--- 静态库文件存放目录
    ├── app_update							--- 应用升级组件
    ├── bootloader							--- 启动引导组件
    ├── bootloader_support					--- 启动引导支持组件
    ├── bt									--- 蓝牙组件
    ├── esp_wifi							--- wifi组件
    ├── driver								--- esp32 soc驱动
    └── ...
```

## 组件目录说明

ESP32平台组件按目录划分，存放在`components`目录下，如下为[NiobeU4开发板](https://gitee.com/openharmony/device_board_openvalley/blob/master/niobeu4/README_zh.md)适配使用到的组件及功能用途说明：

| 组件目录名  | 组件功能用途 |
| :-----------: | :----------------------------------------------------------: |
| `app_update` | 应用升级 |
| `bootloader` | 启动引导 |
| `bootloader_support` | 启动引导加密支持 |
| `bt` | 蓝牙 |
| `cxx` | C++支持 |
| `driver` | 芯片驱动 |
| `efuse` | 一次性可编程存储 |
| `esp32` | esp32平台接口 |
| `esp_adc_cal` | RF ADC校准 |
| `esp_common` | esp公共文件 |
| `esp_event` | 事件增强组件 |
| `esp_hw_support` | 硬件接口支持 |
| `esp_netif` | 网桥组件 |
| `esp_ringbuf` | 环形队列 |
| `esp_rom` | rom包链接文件及头文件 |
| `esp_system` | 系统核心接口(启动、休眠、异常处理等) |
| `esp_timer` | 软件定时器支持 |
| `esp_wifi` | WIFI支持 |
| `esp_lwip` | esp32平台LWIP适配 |
| `osal` | 操作系统抽象层 |
| `hal` | 硬件抽象层 |
| `heap` | esp32堆管理 |
| `log` | 日志组件 |
| `newlib` | newlibC 增强库 |
| `nvs_flash` | 键值对存储 |
| `partition_table` | 分区表 |
| `pthread` | Posix线程增强库 |
| `soc` | 外设资源配置组件 |
| `spi_flash` | SPI Flash操作 |
| `tcpip_adapter` | 网络协议栈适配 |
| `xtensa` | 芯片架构层接口 |



## 二进制文件说明

本目录下涉及到7个静态库文件，均属于乐鑫官方对外公开提供的SDK库文件，具体用途如下:

|           二进制文件            |       文件功能用途        | License授权 |
| :-----------------------------: | :-----------------------: |:-----------------------: |
| `components/libs/libbtdm_app.a` |     蓝牙协议栈库文件      |[EULA](../EULA)|
|   `components/libs/libcore.a`   |      wifi协议内核库       |[EULA](../EULA)|
| `components/libs/libnet80211.a` |      wifi协议库文件       |[EULA](../EULA)|
|   `components/libs/libphy.a`    |     射频物理层库文件      |[EULA](../EULA)|
|    `components/libs/libpp.a`    |      wifi协议内核库       |[EULA](../EULA)|
|   `components/libs/librtc.a`    |  系统时钟、低功耗库文件   |[EULA](../EULA)|
|  `components/libs/libxt_hal.a`  | xtensa架构cpu hal层库文件 |[EULA](../EULA)|

