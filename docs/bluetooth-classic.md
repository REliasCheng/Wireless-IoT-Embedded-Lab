# Classic Bluetooth SPP

## Reference Boundary

Classic Bluetooth SPP 是与 Wi-Fi/TCP/MQTT 并列的参考路径，不是 BLE UART service。当前默认分支不包含 SPP 实现或 vendor radio stack。

```text
Application Data
      ↕
SPP Send / Receive Callback
      ↕
RFCOMM / EDR Stack
      ↕
Native Bluetooth Radio
```

## Interface Model

典型 SPP 集成需要：

- SPP 发送接口。
- 连接状态与发送唤醒回调。
- 数据接收回调。
- 可选 RFCOMM credits 流控。

这些接口仅作为架构模型；仓库没有配对、连接、吞吐或板端数据收发的运行记录。

## 相关入口

- [Wireless Platform](wireless-platform.md)
- [BLE GATT Reference](ble-reference.md)

[上一篇：MQTT & Aliyun IoT](mqtt-and-aliyun.md) · [返回 README](../README.md) · [下一篇：BLE GATT Reference](ble-reference.md)
