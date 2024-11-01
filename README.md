# esp32_esp_now_temple

本项目使用 ESP32 开发板实现基于 ESP-NOW 协议的透传模块。通过串口输入的消息可以被无线发送到指定 ESP32 设备。

## 功能概述

1. 初始化 WiFi 和 ESP-NOW 模块。
2. 向指定 ESP32 设备添加对等设备。
3. 注册 ESP-NOW 的发送回调和接收回调函数。
4. 从串口读取消息,并通过 ESP-NOW 发送到指定设备。
5. 接收到的数据通过串口输出。

## 硬件需求

1. ESP32 开发板 (本项目使用 ESP32-C3)
2. 其他兼容 ESP-NOW 协议的 ESP32 设备

## 软件环境

Platformio 

esp-rduino_2.0.17_idf_4.4.7 内核

## 如何使用

1. 将 `receiverAddress` 修改为目标 ESP32 设备的 MAC 地址。
2. 然后编译&上传

```bash
pio run -t upload
```

## 代码分析

1. `#include <esp_now.h>` 和 `#include <WiFi.h>`: 导入 ESP-NOW 和 WiFi 库。
2. `uint8_t receiverAddress[]`: 指定目标 ESP32 设备的 MAC 地址。
3. `OnDataSent()` 函数: 在数据发送后被调用,用于打印发送状态。
4. `OnDataRecv()` 函数: 在接收到数据时被调用,用于将接收到的数据输出到串口。
5. `setup()` 函数:
   - 初始化串口通信
   - 设置 WiFi 为 STA 模式
   - 初始化 ESP-NOW 模块
   - 注册发送和接收回调函数
   - 添加目标 ESP32 设备为对等设备
   - 发送初始化消息
6. `loop()` 函数:
   - 检查串口是否有可读数据
   - 读取串口输入的消息
   - 通过 ESP-NOW 将消息发送到目标设备

## 注意事项

1. 确保目标 ESP32 设备已经开启,且 MAC 地址正确。
2. 如果发送失败,程序会尝试重新添加对等设备并重试发送。
3. 串口波特率设置为 115200。
4. 该代码仅适用于 ESP32 平台,不能运行在其他微控制器上。

## 参考资源

- [ESP-NOW 协议官方文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [ESP32 Arduino 库文档](https://github.com/espressif/arduino-esp32)
