# 开发环境 | Development Environment

## 平台与工具链

- Target: JieLi AC791N / WL82
- CPU target: pi32v2 R3
- Original toolchain: JieLi pi32v2 Clang/LLVM toolchain
- Original project organization: Makefile and Code::Blocks project files
- Network stack: vendor integration around lwIP
- Runtime: FreeRTOS V9.0.0 through vendor OS abstraction

## 公开快照结构

仓库只保留经许可、凭据和技术边界筛选的应用、配置与接口文件。下列内容不在公开快照内：

- vendor prebuilt libraries, firmware and flashing tools
- complete lwIP / Bluetooth / media / UI product trees
- build outputs and dependency files
- Windows installers and IDE packages
- credential-bearing host examples and TLS test fixtures

## Build Status

当前机器可找到通用 GCC，但未找到 JieLi pi32v2 vendor compiler。公开仓库也刻意不包含完整 SDK 与预编译依赖，因此：

```text
BUILD_PASS=0
BUILD_FAIL=0
BUILD_NOT_AUTOMATED=3 logical projects
```

本阶段没有执行 installer、vendor executable、firmware、烧录、Wi-Fi/Bluetooth 连接或云端连接。

[上一篇：Security Boundaries](security-boundaries.md) · [返回 README](../README.md)
