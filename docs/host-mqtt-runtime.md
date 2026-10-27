# Host MQTT runtime

## Scope and execution

This host application uses official libmosquitto and a separate local Mosquitto Broker over `127.0.0.1` TCP. It is not a JieLi firmware, Wi-Fi, lwIP, Aliyun, or TLS implementation. `device_model.c` is a portable C11 model; `host_mqtt_device.c` owns all libmosquitto calls. The network loop is single-threaded. libmosquitto callbacks execute inside `mosquitto_loop()` on that same thread, so they may update the model without a mutex. Each message payload is borrowed only during its callback; ACK text is formatted into a bounded local buffer, and `mosquitto_publish()` queues its own copy. The process destroys the client only after the loop stops, so no callback remains active.

The Broker harness creates one loopback listener on a random available port. Anonymous access exists only inside that short-lived local test listener. The harness controls only the processes it created and removes its temporary config after each test. No public listener, cloud account, Wi-Fi password or real device credential is used. Evidence logs under `artifacts/` contain generated run IDs and synthetic data.

## Build and test on Ubuntu 24.04

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build pkg-config mosquitto mosquitto-clients libmosquitto-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
ctest --test-dir build --output-on-failure --repeat until-fail:3 -R 'mqtt_(normal_qos1|reconnect)'
```

For the portable model alone, set `-DIOT_ENABLE_NETWORK=OFF`. The full network integration needs the official distribution packages. CI records their exact installed package versions; the project does not invent a source commit for Ubuntu packages.

The bounded device executable is `./build/iot_device --port PORT --run-id RUN --device-id DEVICE --qos 1 --duration-ms 10000`. It connects only to `127.0.0.1`. `tests/mqtt_integration.py` coordinates the Broker, device and independent `mosquitto_sub` / `mosquitto_pub` clients. Its nine scenarios cover `normal_qos0`, `normal_qos1`, `invalid_command`, `reconnect`, `unavailable`, `subscribe_failure`, `publish_failure`, `connect_timeout`, and `client_id_collision`. They have explicit exit codes, test timeouts and retained logs. Run IDs isolate Topics and Client IDs; port selection checks a free loopback port and reports a startup collision instead of killing its owner.

## Protocol

All Topics have the form `lab/<run-id>/<device-id>/<kind>`, where `kind` is `telemetry`, `command`, or `ack`. Each ID is 1–32 ASCII letters, digits, `_` or `-`. The complete Topic is bounded by 127 bytes. MQTT protocol version is 3.1.1 with Clean Session. Subscription uses QoS 1 and is considered active only after successful SUBACK. The demo's `--qos` selects telemetry QoS 0 or 1. ACKs use QoS 1.

Telemetry is synthetic and uses a bounded JSON payload, for example:

```json
{"v":1,"sequence":12,"temperature_c":25.2,"humidity_percent":48,"switch":1}
```

Commands use the **restricted v1 grammar** below, in any key order with optional JSON whitespace:

```json
{"v":1,"request_id":7,"command":"set_switch","value":0}
```

The parser requires each of these four fields exactly once, unsigned decimal integers for `v`, `request_id`, and `value`, `v=1`, `request_id>0`, `value` of 0 or 1, and the literal command `set_switch`. It rejects unknown/duplicate keys, missing fields, wrong types, escapes, non-ASCII strings, invalid UTF-8, overflow, trailing bytes, empty data and payloads over 256 bytes. This is an intentionally limited protocol parser, not a general JSON library. No `sscanf` or unbounded concatenation is used.

ACKs are separate application results, for example:

```json
{"v":1,"request_id":7,"accepted":true,"duplicate":false,"switch":0,"reason":"ok"}
```

A higher request ID applies an idempotent switch assignment. Repeating the latest ID and value yields `accepted=true, duplicate=true` without applying again. The same ID with a different value is `conflict`; a lower ID is `stale`. Invalid messages preserve the switch and use request ID 0 when the ID cannot be parsed. The model counts rejected and duplicate commands separately. No persistence is implemented: deduplication state and outgoing queue do not survive process restart.

The invalid-command integration scenario first publishes a valid command while no ACK subscriber exists, then resends the same request and verifies a duplicate ACK and unchanged switch state. It also sends a command to another run's Topic and checks that this device emits no ACK. The reconnect scenario checks duplicate and stale requests again after a new SUBACK.

## Lifecycle and delivery levels

The client moves through INITIALIZED → CONNECTING → CONNECTED → SUBSCRIBED → RUNNING. A TCP/MQTT failure enters BACKOFF; attempts are 100, 200, 400, then at most 800 ms apart, with a configurable retry limit and 2-second CONNACK and SUBACK deadlines. Reconnection uses Clean Session and therefore submits a new SUBSCRIBE; only its SUBACK restores RUNNING. The harness kills only its own Broker, observes BACKOFF and a failed reconnect attempt, restarts it, waits for the second RUNNING event, then verifies a new Command/ACK. Additional negative fixtures inject one local subscribe or publish failure, hold a silent loopback TCP peer without CONNACK, and create a competing client with the same Client ID; these are explicitly labeled fault tests and are not counted as normal Broker delivery.

`TELEMETRY_QUEUED` means libmosquitto accepted a local publish request. For QoS 1, the publish callback records a protocol completion/PUBACK count. Neither event proves that an independent subscriber processed the data; the harness separately asserts its received Topic/Payload. QoS 0 has no delivery confirmation. QoS 1 is at least once, so the application request ID handles duplicates. No exactly-once end-to-end, offline persistence, real cloud transport, or TLS claim is made.

On SIGTERM, SIGINT or duration expiry the single loop stops, issues DISCONNECT, destroys the client and calls `mosquitto_lib_cleanup()`. The timeout scenario also signals the device while a new connection is pending and requires a clean STOPPED exit. The model's final summary reports queueing, publish callbacks, command counts and reconnect attempts. A retry-limit failure returns nonzero.
