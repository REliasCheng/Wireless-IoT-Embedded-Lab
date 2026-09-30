# Native Wi-Fi Networking

## STA 主线

当前 Wi-Fi task 默认选择 STA mode。公开快照将 AP/STA SSID 和 password 替换为占位符，但保留连接状态机、DHCP 事件和模式切换逻辑。

```text
Native Wi-Fi Start
        ↓
STA Association
        ↓
DHCP Success
        ↓
NET_EVENT_CONNECTED
        ↓
TCP / MQTT Worker
```

`WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC` 标记网络栈已获取地址，TCP 与 MQTT 工作线程在应用中继续检查此状态。这是软件路径证据，不等同于当前环境已完成 Wi-Fi 关联或 DHCP 实测。

## AP / Provisioning Reference

[Wi-Fi reference](../projects/reference/wifi-ap-provisioning/) 保留 AP mode、STA mode、scan 和 provisioning 相关事件入口。它用于标记 SDK 中的模式边界，不将这些路径全部表述为主线已验证功能。

## 架构边界

这里没有外置无线模组或 AT parser。Wi-Fi API 直接连接 vendor network layer 和芯片原生 radio stack。

## 相关文档

- [TCP & lwIP](tcp-and-lwip.md)
- [MQTT & Aliyun IoT](mqtt-and-aliyun.md)

[上一篇：Wireless Platform](wireless-platform.md) · [返回 README](../README.md) · [下一篇：TCP & lwIP](tcp-and-lwip.md)
