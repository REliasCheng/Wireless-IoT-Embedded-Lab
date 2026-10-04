# 开发环境 | Development Environment

## 平台与工具链

- Target: JieLi AC791N / WL82
- CPU target: pi32v2 R3
- Original toolchain: JieLi pi32v2 Clang/LLVM toolchain
- Original project organization: Makefile and Code::Blocks project files
- Network stack: vendor integration around lwIP
- Runtime: FreeRTOS V9.0.0 through vendor OS abstraction

## Current Branch Boundary

当前默认分支只保留原创架构文档与自绘 SVG。下列内容不在当前分支内：

- vendor prebuilt libraries, firmware and flashing tools
- complete lwIP / Bluetooth / media / UI product trees
- build outputs and dependency files
- Windows installers and IDE packages
- credential-bearing host examples and TLS test fixtures

## Build Status

当前仓库没有完整 SDK、vendor compiler、工程文件或预编译依赖，因此：

```text
BUILD_PASS=0
BUILD_FAIL=0
BUILD_NOT_AVAILABLE=YES
```

本阶段没有执行 installer、vendor executable、firmware、烧录、Wi-Fi/Bluetooth 连接或云端连接。

[上一篇：Security Boundaries](security-boundaries.md) · [返回 README](../README.md)
