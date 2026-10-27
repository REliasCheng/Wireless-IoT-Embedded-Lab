# 无线平台 | Wireless Platform

## 平台边界

参考平台是 JieLi AC791N/WL82，CPU 目标为 pi32v2 R3。架构模型使用芯片与 vendor SDK 的 Native Wi-Fi/Bluetooth 能力，不是 MCU 经 UART 控制外置 AT 模组的结构。

```text
Application
    ├── MQTT / Socket → lwIP → Native Wi-Fi
    └── SPP           → EDR  → Native Bluetooth
                                   ↓
                            AC791N / WL82
```

## Current Boundary

当前默认分支已有独立的 **Host loopback MQTT 应用**，见 [Host MQTT runtime](host-mqtt-runtime.md)。它不包含 JieLi 目标应用、SDK、radio binary 或 vendor toolchain。Native Wi-Fi、Classic Bluetooth 与 BLE 的启用状态仍须在未来目标实现中明确配置；本文其余部分只定义历史技术路线。

## 与其他仓库的边界

- ARM 仓库负责 MCU 外设与固件层。
- FreeRTOS 仓库负责调度、IPC 与同步机制。
- 本仓库仅解释 OS 如何承载无线事件和网络工作线程，重点是 connectivity 与 device/cloud data path。

## 相关入口

- [Wi-Fi Networking](wifi-networking.md)
- [Bluetooth Classic](bluetooth-classic.md)

[返回 README](../README.md) · [下一篇：Native Wi-Fi Networking](wifi-networking.md)
