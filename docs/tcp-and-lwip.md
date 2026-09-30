# TCP 与 lwIP | TCP & lwIP

## 数据通路

[TCP client](../projects/01-connectivity-mainline/course/itheima/itheima_tcp_client_demo.c) 通过 SDK socket wrapper 完成：

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

MQTT client 的 network adapter 同样建立在 TCP 连接之上。本仓库的 MQTT 主线目标端口为 1883，没有 TLS transport 进入当前路径的证据。

## 快照限制

完整 lwIP 源码和 vendor network binary 未进入仓库。选中文件用于分析应用、socket 接口和无线事件之间的关系，不是脱离 SDK 的 standalone build。

## 工程入口

- [Connectivity Mainline](../projects/01-connectivity-mainline/)

[上一篇：Native Wi-Fi Networking](wifi-networking.md) · [返回 README](../README.md) · [下一篇：MQTT & Aliyun IoT](mqtt-and-aliyun.md)
