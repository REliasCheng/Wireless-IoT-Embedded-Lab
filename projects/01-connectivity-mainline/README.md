# Connectivity Mainline

## 定位

该目录保留 DevKitBoard connectivity family 的最小应用/配置/接口快照，用于阅读 Native Wi-Fi STA、lwIP TCP、MCU-side MQTT/Aliyun 和 Classic Bluetooth SPP 之间的关系。

## 技术入口

- [`app_main.c`](course/apps/demo/demo_DevKitBoard/app_main.c)：task table 与 network event dispatch。
- [`wifi_demo_task.c`](course/apps/demo/demo_DevKitBoard/wifi_demo_task.c)：Wi-Fi mode/state 与 DHCP event；凭据字面量已替换。
- [`itheima_tcp_client_demo.c`](course/itheima/itheima_tcp_client_demo.c)：Wi-Fi ready 后的 TCP client 工作线程。
- [`itheima_mqtt_demo.c`](course/itheima/itheima_mqtt_demo.c)：MQTT Connect/Subscribe/Publish/Yield/reconnect；云端连接身份已替换。
- [`spp_trans_data.c`](course/apps/demo/demo_DevKitBoard/spp_trans_data.c)：Classic Bluetooth SPP 数据通道。
- [`app_config.h`](course/apps/demo/demo_DevKitBoard/include/app_config.h) / [`demo_config.h`](course/apps/demo/demo_DevKitBoard/include/demo_config.h)：当前特性开关。

## 快照边界

- 该目录不是完整 SDK，不能独立构建。
- Wi-Fi 和 MQTT 文件为 `SANITIZED_DERIVATIVE`，仅替换凭据/连接身份字面量。
- temperature / humidity payload 字段由软件变量产生，不是传感器数据。
- 没有当前 build pass、板端运行、无线连接或云端上线证据。

[返回根 README](../../README.md)
