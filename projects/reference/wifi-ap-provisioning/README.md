# Wi-Fi AP & Provisioning Reference

## 作用

该快照保留 official Wi-Fi demo 的 STA/AP 模式、scan、DHCP 和 provisioning 事件入口，用于补充主线 STA 之外的 SDK 能力边界。

## 关键文件

- [`app_main.c`](course/apps/demo/demo_wifi/app_main.c)
- [`app_config.h`](course/apps/demo/demo_wifi/include/app_config.h)
- [`wifi_demo_task.c`](course/apps/demo/demo_wifi/wifi_demo_task.c)

## 边界

`wifi_demo_task.c` 的 SSID、password 和 AirKiss example key 已改为公开占位值。该目录是架构参考，不表示 AP/provisioning 已在当前硬件上验证。

[Native Wi-Fi Networking 文档](../../../docs/wifi-networking.md) · [Security Boundaries](../../../docs/security-boundaries.md) · [返回根 README](../../../README.md)
