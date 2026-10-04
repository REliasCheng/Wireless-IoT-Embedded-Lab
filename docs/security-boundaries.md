# 安全边界 | Security Boundaries

## Transport Boundary

文档主线限定为 plain MQTT over TCP 1883。当前默认分支不包含 TLS/HTTPS 组件，也不能支持 encrypted MQTT 或 certificate validation 的结论。

## Credentials

- 当前默认分支不包含 Wi-Fi SSID/password、MQTT broker address、username、password、client ID 或 topics。
- 当前默认分支不包含 SDK TLS/private-key test fixtures、host-side credential examples 或 editor recovery files。
- 旧提交仍可能保留历史风险；本次普通提交没有改写历史。

## Evidence Boundary

架构文档说明协议层次，但不证明：

- 设备已经云端上线。
- MQTT Publish/Subscribe 已经本轮实机验证。
- Bluetooth SPP 或 BLE 连接已验证。
- 传输链路已加密。
- 凭据生命周期或安全存储已实现。

[上一篇：FreeRTOS Integration](freertos-integration.md) · [返回 README](../README.md) · [下一篇：Development Environment](development-environment.md)
