# FreeRTOS Integration

## Runtime 关系

无线 SDK 常通过 vendor OS abstraction 承载 Wi-Fi、TCP 或 MQTT 工作线程。本文只描述任务边界，不提供 FreeRTOS、vendor abstraction 或应用实现。

```text
Wi-Fi / Runtime Event
        ↓
Vendor OS Abstraction
        ↓
Network Worker Thread
        ↓
TCP / MQTT Processing
        ↓
RTOS Kernel Context
```

## Task Boundary

- Network event dispatch 应把连接状态转换为明确的任务事件。
- TCP worker 应在网络就绪后创建和管理 socket。
- MQTT worker 应负责会话、订阅、发布和重连状态。

这里不重复 RTOS scheduler、Queue 或 Semaphore 基础，只标出 RTOS 如何承载无线栈与应用工作线程。

## 验证状态

当前没有运行时 task trace、调度记录、无线稳定性数据或可构建 vendor project。

[上一篇：BLE GATT Reference](ble-reference.md) · [返回 README](../README.md) · [下一篇：Security Boundaries](security-boundaries.md)
