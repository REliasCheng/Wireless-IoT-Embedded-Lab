# TCP 与 lwIP | TCP & lwIP

## 数据通路

典型 TCP client 通过 SDK socket wrapper 完成：

1. 等待 Wi-Fi STA 进入 DHCP success 状态。
2. 注册 IPv4 TCP socket。
3. 向配置的 server address / port 发起连接。
4. 在工作线程中调用 send / receive。
5. 出错或退出时注销 socket。

```text
Wi-Fi STA Ready
      ↓
lwIP / SDK Socket Wrapper
      ↓
TCP Connect
      ↓
Application Send / Receive
```

## 与 MQTT 的关系

MQTT client 的 network adapter 同样建立在 TCP 连接之上。本文档只描述 plain TCP 1883 路径，没有 TLS transport 实现或证据。

## 快照限制

完整 lwIP 源码和 vendor network binary 未进入当前默认分支。新的 [Host MQTT 应用](host-mqtt-runtime.md)可独立构建并通过回环 TCP 联调，但不使用 JieLi socket wrapper 或 lwIP，也不证明 Wi-Fi 网络就绪。

[上一篇：Native Wi-Fi Networking](wifi-networking.md) · [返回 README](../README.md) · [下一篇：MQTT & Aliyun IoT](mqtt-and-aliyun.md)
