# MQTT 与 Aliyun IoT

## MCU-side Client

架构模型在 MCU 侧运行 MQTT client，通过 network adapter 和芯片原生 Wi-Fi 建立 plain TCP 1883 连接，再执行 Connect、Subscribe、Publish、Yield 和 reconnect。它不是外置模组 AT firmware 代理模型。

```text
Wi-Fi STA / DHCP Ready
          ↓
TCP Connect :1883
          ↓
MQTT CONNECT
          ↓
SUBSCRIBE (QoS 1) / PUBLISH (QoS 1)
          ↓
MQTT Yield / Reconnect
```

## Application Payload

示例数据模型使用 temperature、humidity 和 switch-like state 字段。它们只表示合成应用变量，不被描述为真实传感器采样值。

```text
Synthetic Application Variables
          ↓
JSON Payload
          ↓
MQTT Publish Topic
          ↓
Aliyun IoT
```

## 凭据边界

当前默认分支不包含 broker address、username、password、client ID、topic、certificate 或 private key。未来实现应通过安全配置注入这些值，不应提交真实凭据。

## 安全边界

当前仓库没有 MQTTS、证书校验或 TLS session 证据。详见 [Security Boundaries](security-boundaries.md)。

[上一篇：TCP & lwIP](tcp-and-lwip.md) · [返回 README](../README.md) · [下一篇：Classic Bluetooth SPP](bluetooth-classic.md)
