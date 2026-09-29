# BLE GATT Reference

## Reference 定位

BLE 代码来自独立的 official demo。该 demo 的配置启用 BLE、关闭 Classic Bluetooth；DevKitBoard 主线则相反，因此 BLE 不属于主线当前启用配置。

## GATT 路径

[ble.c](../projects/reference/ble-gatt/course/apps/demo/demo_ble/bt_ble/ble.c) 包含：

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

这些文件用于说明 SDK 的 BLE GATT 端口和主线配置边界，不代表 BLE 连接、手机交互或板端行为已验证。

## 工程入口

- [BLE GATT Reference](../projects/reference/ble-gatt/)
