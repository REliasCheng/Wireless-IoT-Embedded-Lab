# Classic Bluetooth SPP

## 主线状态

DevKitBoard 选中配置启用 Classic Bluetooth，关闭 BLE，并打开 SPP transport。该路径是 AC791N/WL82 原生 EDR stack 上的串口协议数据通道，不是 BLE UART service。

```text
Application Data
      ↕
SPP Send / Receive Callback
      ↕
RFCOMM / EDR Stack
      ↕
Native Bluetooth Radio
```

## 工程关系

[spp_trans_data.c](../projects/01-connectivity-mainline/course/apps/demo/demo_DevKitBoard/spp_trans_data.c) 保留：

- SPP 发送接口。
- 连接状态与发送唤醒回调。
- 数据接收回调。
- 可选 RFCOMM credits 流控。

代码路径表明协议接口存在；仓库没有附带配对、连接、吞吐或板端数据收发的运行记录。

## 相关入口

- [Wireless Platform](wireless-platform.md)
- [BLE GATT Reference](ble-reference.md)
