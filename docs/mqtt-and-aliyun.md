# MQTT 与 Aliyun IoT

## MCU-side Client

主线在 MCU 侧初始化 MQTT client，通过 network adapter 建立 TCP 1883 连接，再执行 Connect、Subscribe、Publish、Yield 和 reconnect。MQTT 不是由外置模组的 AT firmware 代理。

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

[MQTT application](../projects/01-connectivity-mainline/course/itheima/itheima_mqtt_demo.c) 将 temperature、humidity 和 switch-like state 写入 JSON payload。temperature 与 humidity 是循环中递增的软件变量，不是传感器采样值。

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

原始文件含 broker-derived address、username、password、client ID 和 topic 字面量。公开文件仅将这些字面量替换为 `YOUR_*` 占位符；函数、分支、协议调用和 payload 逻辑保持不变。差异由 [SOURCE_SELECTION_MANIFEST.csv](../SOURCE_SELECTION_MANIFEST.csv) 和 [MIGRATION_HASH_VERIFICATION.csv](../MIGRATION_HASH_VERIFICATION.csv) 记录。

## 安全边界

当前主线没有 MQTTS、证书校验或 TLS session 证据。详见 [Security Boundaries](security-boundaries.md)。
