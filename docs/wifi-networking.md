# Native Wi-Fi Networking

## STA Model

STA 模型包含启动、关联、DHCP 与网络就绪事件。当前默认分支不包含 task 实现、SSID/password 或 vendor SDK。

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

具体 vendor event 名称和状态检查方式取决于 SDK。架构路径不等同于当前环境已完成 Wi-Fi 关联或 DHCP 实测。

## AP / Provisioning Reference

AP mode、scan 和 provisioning 仅作为参考能力，不被表述为当前已实现或已验证功能。

## 架构边界

这里没有外置无线模组或 AT parser。Wi-Fi API 直接连接 vendor network layer 和芯片原生 radio stack。

## 相关文档

- [TCP & lwIP](tcp-and-lwip.md)
- [MQTT & Aliyun IoT](mqtt-and-aliyun.md)

[上一篇：Wireless Platform](wireless-platform.md) · [返回 README](../README.md) · [下一篇：TCP & lwIP](tcp-and-lwip.md)
