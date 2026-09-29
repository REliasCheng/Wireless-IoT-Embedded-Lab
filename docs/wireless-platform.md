# 无线平台 | Wireless Platform

## 平台边界

主线平台是 JieLi AC791N/WL82，CPU 目标为 pi32v2 R3。Wi-Fi 与 Bluetooth 由芯片及 vendor SDK 原生提供，不存在 MCU 经 UART 控制 ESP8266 类 AT 模组的主线结构。

```text
Application
    ↓
MQTT / Socket / SPP API
    ↓
lwIP / Bluetooth Stack
    ↓
Native Wi-Fi / Bluetooth Radio
    ↓
AC791N / WL82
```

## 当前配置

[DevKitBoard configuration](../projects/01-connectivity-mainline/course/apps/demo/demo_DevKitBoard/include/app_config.h) 显示：

- Classic Bluetooth 开启。
- SPP 数据通道开启。
- BLE 关闭。
- Wi-Fi demo 和 MCU-side MQTT 应用入口存在。

仓库保留的是应用与接口快照，不包含完整 SDK、radio binary 或 vendor toolchain，因此不将源文件存在解释为当前构建或板端运行证据。

## 与其他仓库的边界

- ARM 仓库负责 MCU 外设与固件层。
- FreeRTOS 仓库负责调度、IPC 与同步机制。
- 本仓库仅解释 OS 如何承载无线事件和网络工作线程，重点是 connectivity 与 device/cloud data path。

## 相关入口

- [Connectivity Mainline](../projects/01-connectivity-mainline/)
- [Wi-Fi Networking](wifi-networking.md)
- [Bluetooth Classic](bluetooth-classic.md)
