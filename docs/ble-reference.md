# BLE GATT Reference

## Reference 定位

BLE GATT 仅作为并列技术参考，不属于本仓库的 Native Wi-Fi → lwIP → TCP → MQTT 主线。当前默认分支不包含 BLE demo 或 vendor SDK。

## GATT 路径

典型 BLE client 路径包含：

- BLE client 初始化。
- 设备名称和 UUID 匹配。
- Characteristic Read 结果事件。
- Notification / Indication 事件。
- Write Without Response 发送入口。

```text
Scan / Match
     ↓
GATT Service and Characteristic Discovery
     ↓
Read / Write / Notify / Indicate
```

这些接口名称只说明协议角色，不代表 BLE 连接、手机交互或板端行为已实现或验证。

[上一篇：Classic Bluetooth SPP](bluetooth-classic.md) · [返回 README](../README.md) · [下一篇：FreeRTOS Integration](freertos-integration.md)
