# 安全边界 | Security Boundaries

## Transport Boundary

选中 MQTT 主线使用 plain MQTT over TCP 1883。SDK 其他目录中存在 TLS/HTTPS 组件，但它们没有进入当前公开主线，也不能支持 encrypted MQTT 或 certificate validation 的结论。

## Credentials

- Wi-Fi SSID/password 在公开快照中改为 `YOUR_*` 占位符。
- MQTT broker-derived address、username、password、client ID 和 topics 改为 `YOUR_*` 占位符。
- 原始凭据不进入 Git 历史。
- SDK TLS/private-key test fixtures、host-side credential examples 和 editor recovery files 不进入公开仓库。

## Evidence Boundary

公开文件证明连接逻辑和协议接口存在，不证明：

- 设备已经云端上线。
- MQTT Publish/Subscribe 已经本轮实机验证。
- Bluetooth SPP 或 BLE 连接已验证。
- 传输链路已加密。
- 凭据生命周期或安全存储已实现。

[上一篇：FreeRTOS Integration](freertos-integration.md) · [返回 README](../README.md) · [下一篇：Development Environment](development-environment.md)
