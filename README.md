# 无线与物联网嵌入式实验室
## Wireless & IoT Embedded Lab

基于 JieLi AC791N / WL82 原生 Wi-Fi 与 Bluetooth 平台，围绕 lwIP、TCP、MQTT、Aliyun IoT 和 Classic Bluetooth SPP 数据通路组织的嵌入式无线通信仓库。

<p align="center">
  <img src="assets/images/architecture/wireless-connectivity-stack.svg" alt="Wireless connectivity stack" width="900">
</p>

**Platform:** AC791N / WL82 · **CPU:** pi32v2 R3 · **Network:** lwIP / TCP · **Messaging:** MQTT / TCP 1883 · **Runtime:** FreeRTOS V9.0.0

## 👋 项目简介 | Overview

AC791N / WL82 在芯片与 vendor SDK 内提供 Wi-Fi 和 Bluetooth 能力，不采用 MCU 经 UART 控制外置 Wi-Fi 模组的 AT 架构。公开主线保留 Native Wi-Fi STA、lwIP TCP、MCU-side MQTT、Aliyun IoT 数据通路以及 Classic Bluetooth EDR/SPP；BLE GATT 与 Wi-Fi AP/provisioning 作为独立 reference。

仓库是经过许可、凭据和技术边界筛选的应用与接口快照，不包含完整 vendor SDK，也不能脱离 JieLi 工具链独立构建。公开派生文件中的连接凭据已替换为占位符。

## ⚙ 技术范围 | Technical Scope

### 📡 Native Wireless

- JieLi AC791N / WL82, pi32v2 R3
- Native Wi-Fi STA and DHCP events
- Classic Bluetooth EDR / SPP enabled in the mainline

### 🌐 Network Stack

- lwIP socket interface
- TCP client and network-ready state
- Native radio path without an external AT-command module

### ☁ MQTT & IoT

- MCU-side MQTT client
- Publish / Subscribe, QoS 1, Keep Alive and reconnect flow
- Aliyun IoT topic and application payload path over plain TCP port 1883

### 🧪 References

- BLE GATT client reference; disabled in the mainline configuration
- Wi-Fi AP, scan and provisioning reference

## 📡 原生无线平台 | Native Wireless

主线应用通过芯片原生网络与 Bluetooth stack 建立两条通信路径：

```text
Application
    ├── MQTT → TCP / lwIP → Native Wi-Fi → Aliyun IoT
    └── SPP  → Classic Bluetooth EDR → Peer Device
```

BLE GATT 只保留为 SDK reference，不属于 DevKitBoard 主线当前启用配置。平台与配置证据见 [Wireless Platform](docs/wireless-platform.md)。

## 🌐 TCP 与 MQTT | TCP & MQTT

Wi-Fi STA 获得 DHCP 地址后，应用工作线程通过 lwIP socket wrapper 建立 TCP 连接；MQTT client 在同一路径上执行 Connect、Subscribe、Publish、Yield 和 reconnect。

- [Native Wi-Fi Networking](docs/wifi-networking.md)
- [TCP & lwIP](docs/tcp-and-lwip.md)
- [MQTT & Aliyun IoT](docs/mqtt-and-aliyun.md)

## 🔵 Bluetooth | Bluetooth

Classic Bluetooth EDR/SPP 是主线启用的数据通道，与 MQTT/TCP 路径并列；它不是 BLE UART service。BLE client 的 UUID 匹配、Characteristic Read/Write 与 Notify/Indicate 保留在 [BLE GATT Reference](docs/ble-reference.md) 中。

## ☁ IoT 数据通路 | IoT Data Flow

<p align="center">
  <img src="assets/images/diagram/mqtt-data-flow.svg" alt="MQTT application data flow" width="900">
</p>

示例 payload 中的 temperature / humidity 字段来自循环递增的软件变量，只用于呈现 JSON → MQTT → TCP → Aliyun IoT 数据通路，不是传感器采样证据。主线使用 plain MQTT over TCP 1883；当前没有 TLS session 或云端在线运行记录。

## 🚀 核心工程 | Featured Entries

### [Mainline Connectivity Project](projects/01-connectivity-mainline/)

DevKitBoard connectivity 主线：Native Wi-Fi STA、DHCP、lwIP TCP、MCU-side MQTT/Aliyun 与 Classic EDR/SPP。

`Native Wi-Fi / lwIP / TCP / MQTT / Aliyun IoT / Classic SPP`

### [BLE GATT Reference](projects/reference/ble-gatt/)

官方 BLE client reference，保留 UUID 匹配、Characteristic Read/Write 与 Notification/Indication 入口。

`BLE / GATT / Reference Only`

### [Wi-Fi AP & Provisioning Reference](projects/reference/wifi-ap-provisioning/)

官方 Wi-Fi reference，补充 AP、scan 与 provisioning 的模式和事件入口。

`Wi-Fi AP / Scan / Provisioning / Reference Only`

## 📂 仓库结构 | Repository Structure

```text
Wireless-IoT-Embedded-Lab/
├── assets/images/           # 自有连接架构与数据流图
├── docs/                    # 无线、网络、MQTT、RTOS 与安全边界
├── projects/
│   ├── 01-connectivity-mainline/
│   └── reference/
│       ├── ble-gatt/
│       └── wifi-ap-provisioning/
├── third_party/licenses/    # 已选第三方文件对应的许可证据
├── SOURCE_SELECTION_MANIFEST.csv
└── MIGRATION_HASH_VERIFICATION.csv
```

## 🛠 开发环境 | Development Environment

- JieLi AC791N / WL82 platform
- pi32v2 R3 vendor Clang/LLVM toolchain
- Original Makefile / Code::Blocks project organization
- FreeRTOS V9.0.0 through vendor OS abstraction

当前机器未发现 JieLi pi32v2 工具链，3 个逻辑工程均未执行自动构建；本阶段也未执行烧录、无线连接或 Aliyun IoT 连接。详见 [Development Environment](docs/development-environment.md)。

## 📖 技术文档 | Documentation

- [Wireless Platform](docs/wireless-platform.md)
- [Native Wi-Fi Networking](docs/wifi-networking.md)
- [TCP & lwIP](docs/tcp-and-lwip.md)
- [MQTT & Aliyun IoT](docs/mqtt-and-aliyun.md)
- [Classic Bluetooth SPP](docs/bluetooth-classic.md)
- [BLE GATT Reference](docs/ble-reference.md)
- [FreeRTOS Integration](docs/freertos-integration.md)
- [Security Boundaries](docs/security-boundaries.md)
- [Development Environment](docs/development-environment.md)

## 📜 来源与许可 | License

仓库新编写的 README、技术文档和 SVG 适用根目录 [LICENSE](LICENSE)。选中的 vendor / third-party 文件继续受其原始许可条款和文件头约束，详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
