# Wireless-IoT-Embedded-Lab

面向嵌入式设备的 Native Wi-Fi、lwIP、TCP、MQTT 与 IoT Integration 架构文档实验。

**📡 Connectivity Stack**

![Embedded connectivity path](assets/images/architecture/portfolio-overview.svg)

## Connectivity Snapshot

| Connectivity Focus | Current Scope |
| --- | --- |
| Repository Type | Wireless / IoT Architecture Lab |
| Reference Platform | JieLi AC791N / WL82 |
| Network Path | Native Wi-Fi → lwIP → TCP → MQTT → IoT Integration |
| Security Boundary | Plain TCP 1883 reference；no TLS / MQTTS claim |
| Public Implementation | Not included in the current default branch |
| Verification | Architecture review；build, connection and runtime evidence not provided |

## 📌 Overview

仓库以原创文档和 SVG 描述 Native Wi-Fi STA、DHCP/lwIP、TCP socket、MQTT client 与设备侧 IoT 数据通路。Classic Bluetooth EDR/SPP 作为并列技术路径，BLE GATT、Wi-Fi AP、scan 和 provisioning 只作为 Reference。

当前默认分支不分发 JieLi SDK、`itheima` 课程代码、Paho 接口副本、FreeRTOS 源码、证书、密钥或其他 vendor 文件。软件变量示例不被描述为真实传感器数据，plain MQTT 不被描述为 TLS/MQTTS。

## 🏗️ Architecture

![Wireless connectivity stack](assets/images/architecture/wireless-connectivity-stack.svg)

```text
Native Wi-Fi
      ↓
lwIP
      ↓
TCP
      ↓
MQTT
      ↓
IoT Integration
```

## ✨ Key Features

| Capability | Documentation Entry |
| --- | --- |
| Platform boundary | [Wireless Platform](docs/wireless-platform.md) |
| Wi-Fi state model | [Wi-Fi Networking](docs/wifi-networking.md) |
| TCP/lwIP relationship | [TCP and lwIP](docs/tcp-and-lwip.md) |
| MQTT transport boundary | [MQTT and Aliyun IoT](docs/mqtt-and-aliyun.md) |
| Bluetooth distinction | [Classic Bluetooth](docs/bluetooth-classic.md) and [BLE Reference](docs/ble-reference.md) |
| Credential and TLS boundary | [Security Boundaries](docs/security-boundaries.md) |

## 📂 Project Structure

```text
Wireless-IoT-Embedded-Lab/
├── README.md
├── LICENSE
├── THIRD_PARTY_NOTICES.md
├── assets/images/  # Repository-authored architecture and flow SVG
└── docs/           # Wireless and IoT architecture documentation
```

## 📚 Documentation

- [Wireless Platform](docs/wireless-platform.md)
- [Wi-Fi Networking](docs/wifi-networking.md)
- [TCP and lwIP](docs/tcp-and-lwip.md)
- [MQTT and Aliyun IoT](docs/mqtt-and-aliyun.md)
- [Classic Bluetooth](docs/bluetooth-classic.md)
- [BLE Reference](docs/ble-reference.md)
- [FreeRTOS Integration](docs/freertos-integration.md)
- [Security Boundaries](docs/security-boundaries.md)
- [Development Environment](docs/development-environment.md)

## 🧪 Verification

| Verification Layer | Status | Boundary |
| --- | --- | --- |
| Host Test | NOT PROVIDED | No public network implementation or test harness |
| Build Verification | NOT PROVIDED | No current pi32v2 project or SDK source is distributed |
| Hardware Validation | NOT PROVIDED | No reviewable Wi-Fi or Bluetooth connection record |
| Runtime Evidence | NOT PROVIDED | No DHCP, TCP, MQTT, cloud, or radio log |

## License Boundary

根目录 MIT License 仅覆盖当前默认分支中仓库维护者编写的文档、配置与自绘 SVG。JieLi SDK、Paho、FreeRTOS、课程代码、credentials、certificates 和 vendor 文件未包含在当前默认分支；历史提交风险单独保留。详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
