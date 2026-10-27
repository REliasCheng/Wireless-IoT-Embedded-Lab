# Third-Party Notices

## Eclipse Mosquitto

- Official upstream: https://github.com/eclipse-mosquitto/mosquitto and https://mosquitto.org/
- Components: Mosquitto Broker, libmosquitto C client library, `mosquitto_pub` and `mosquitto_sub` test clients.
- Integration: Ubuntu 24.04 distribution packages `mosquitto`, `mosquitto-clients`, `libmosquitto-dev`; CI prints their actual installed versions. No upstream source or binaries are committed to this repository.
- Upstream license: Eclipse Public License 2.0 OR Eclipse Distribution License 1.0 (SPDX `EPL-2.0 OR BSD-3-Clause`). The distribution retains the corresponding upstream notices.

## Repository code and exclusions

The device model, restricted JSON protocol parser, Host MQTT adapter, test harness, CI and new documentation are original repository work under its root MIT license. That license does not relicense Mosquitto or any historical vendor/course material.

No JieLi SDK, Paho source, FreeRTOS source, `itheima` course application, unknown-rights asset, cloud account configuration, Wi-Fi password, certificate or private key is imported. Historic commits have not been rewritten; their separate provenance risks remain separate from this Host foundation. See [source provenance](docs/source-provenance.md).
