# FreeRTOS Integration

## Runtime 关系

源快照中的 FreeRTOS 版本头为 V9.0.0。无线应用通过 vendor OS abstraction 工作，应用文件调用 `thread_fork()` 创建 Wi-Fi、TCP 或 MQTT 工作线程；公开应用层不直接展开内核调度接口。

```text
Wi-Fi / Runtime Event
        ↓
Vendor OS Abstraction
        ↓
Network Worker Thread
        ↓
TCP / MQTT Processing
        ↓
FreeRTOS V9.0.0 Kernel Context
```

## 工程关系

- [app_main.c](../projects/01-connectivity-mainline/course/apps/demo/demo_DevKitBoard/app_main.c) 保留 task table 和 network event dispatch。
- [TCP client](../projects/01-connectivity-mainline/course/itheima/itheima_tcp_client_demo.c) 在独立工作线程中等待 Wi-Fi ready 并处理 socket。
- [MQTT application](../projects/01-connectivity-mainline/course/itheima/itheima_mqtt_demo.c) 创建网络等待与 MQTT 处理线程。

这里不重复 FreeRTOS scheduler、Queue 或 Semaphore 基础。仓库只标出 RTOS 如何承载无线栈与应用工作线程。

## 验证状态

当前没有运行时 task trace、调度记录或无线稳定性数据，也未在本机器上自动构建 vendor project。

[上一篇：BLE GATT Reference](ble-reference.md) · [返回 README](../README.md) · [下一篇：Security Boundaries](security-boundaries.md)
