# Linux to ESP-NOW Communication Bridge

这个项目演示了如何使用 Linux 原始套接字向基于 ESP-NOW 协议的设备发送自定义的 IEEE 802.11 数据包,实现跨平台的点对点通信。

## 功能概述

1. 创建一个 Linux 原始套接字,用于发送自定义的 IEEE 802.11 数据包。
2. 构建包含 IEEE 802.11 无线网络数据包头部信息和 WLAN 数据包字段的数据包结构体。
3. 填充数据包头部和有效载荷,模拟 ESP-NOW 设备能够接收和处理的数据格式。
4. 循环发送构建好的数据包,实现与 ESP-NOW 设备的通信。

## 使用说明

1. 确保目标 ESP-NOW 设备已经配置好并正在运行。
2. 编译代码:
   ```
   g++ xxx.cpp -o xxx
   ```
3. 运行程序,并指定要使用的网络接口:
   ```
   sudo ./xxx <interface>
   ```
   替换 `<interface>` 为您要使用的网络接口,例如 `eth0` 或 `wlan0`。
4. 程序会开始循环发送 IEEE 802.11 数据包。
5. 同时 运行 一个 esp32上面跑 espnow监听 或者 发送 然后看日志

## 代码分析

1. `packetSetUp()` 函数用于填充 `espPacket` 结构体,设置数据包头部和有效载荷信息。
2. `create_raw_socket()` 函数用于创建原始套接字,并绑定到指定的网络接口。
3. `main()` 函数中:
   - 创建原始套接字。
   - 在一个无限循环中,调用 `packetSetUp()` 函数填充数据包,然后使用 `sendto()` 函数通过原始套接字发送数据包。
   - 在发送之前,会打印出完整的数据包十六进制表示和有效载荷内容。
   - 发送完成后,会暂停 1 秒钟后再发送下一个数据包。

## 注意事项

1. 确保目标 ESP-NOW 设备的配置与发送的数据包格式相匹配,包括 MAC 地址、无线频道等。
2. 该程序需要以 `sudo` 权限运行,以获取原始套接字所需的权限。
3. 在实际使用中,请遵守相关的法律法规,避免造成危害。

## 参考资源

- [原始套接字编程](https://man7.org/linux/man-pages/man7/raw.7.html)
- [IEEE 802.11 无线网络协议](https://en.wikipedia.org/wiki/IEEE_802.11)
- [ESP-NOW 协议](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)

