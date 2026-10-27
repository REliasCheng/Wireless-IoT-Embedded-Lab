# 开发环境 | Development Environment

## 平台与工具链

- Target: JieLi AC791N / WL82
- CPU target: pi32v2 R3
- Original toolchain: JieLi pi32v2 Clang/LLVM toolchain
- Original project organization: Makefile and Code::Blocks project files
- Network stack: vendor integration around lwIP
- Runtime: FreeRTOS V9.0.0 through vendor OS abstraction

## Current Host build

当前默认分支包含可移植 C11 设备模型与 Ubuntu 24.04 Host MQTT 工程。CMake 使用系统 `libmosquitto-dev` 链接真实客户端，Python harness 启动本轮测试 Broker 并以独立 CLI 客户端核对报文；详见 [Host MQTT runtime](host-mqtt-runtime.md)。Windows 本地可使用 `-DIOT_ENABLE_NETWORK=OFF` 验证无网络依赖的业务核心。

## Target branch boundary

JieLi 目标仍只保留原创架构文档与自绘 SVG；下列目标组件不在当前分支内：

- vendor prebuilt libraries, firmware and flashing tools
- complete lwIP / Bluetooth / media / UI product trees
- build outputs and dependency files
- Windows installers and IDE packages
- credential-bearing host examples and TLS test fixtures

## Build Status

当前仓库没有完整 JieLi SDK、vendor compiler、工程文件或预编译依赖，因此以下状态仅针对 **JieLi 目标构建**：

```text
BUILD_PASS=0
BUILD_FAIL=0
BUILD_NOT_AVAILABLE=YES
```

Host 工程可以在 Ubuntu 构建与回环 Broker 联调；本阶段没有执行 installer、vendor executable、firmware、烧录、Wi-Fi/Bluetooth 连接或云端连接。AC7916 与 AC791N/WL82 的目标等同性尚无充分证据，见 [source provenance](source-provenance.md)。

[上一篇：Security Boundaries](security-boundaries.md) · [返回 README](../README.md)
