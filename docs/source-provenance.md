# Source provenance and target identity

| Material reviewed | Classification | Decision |
| --- | --- | --- |
| Existing repository architecture text and diagrams | USER_ORIGINAL | Retained; updated Host claims only where implementation now exists. |
| New device model, host adapter, harness and CI | USER_ORIGINAL | Original implementation under this repository's MIT license. |
| Official Eclipse Mosquitto Broker, libmosquitto and CLI clients | LICENSED_REUSABLE | Ubuntu 24.04 system packages; source is not copied into this repository. Exact installed versions are emitted in CI. Upstream dual license: EPL-2.0 OR EDL-1.0 (BSD-3-Clause SPDX). |
| Historic JieLi SDK-matched AC791N/WL82 subset | REFERENCE_ONLY | Original local audit identified an Apache-2.0 boundary for the matched vendor subset; no SDK source is imported here. |
| Historic Paho Embedded C headers and FreeRTOS V9 interfaces | REFERENCE_ONLY | Historical audit found incomplete notice sets; no files are imported here. This Host implementation uses libmosquitto instead of Paho. |
| Course `itheima` TCP/MQTT application and sanitized credential derivatives | RIGHTS_UNVERIFIED | Excluded. Redacting secrets did not establish redistribution rights. |
| Historic build outputs and archives | BUILD_EVIDENCE_ONLY | Do not establish this Host run, JieLi target compatibility or radio/cloud behavior. |

The reviewed course mainline and classification refer to **AC791N / WL82** and a pi32v2 R3 toolchain. No matching source configuration, chip documentation or board record in the reviewed handoff establishes that **AC7916** is the same target or is pin/SDK/toolchain compatible. `TARGET_MODEL_UNRESOLVED=YES`. A target project requires the exact chip marking and board revision, official SDK/DFP or equivalent matching dependency, compiler and linker configuration, pin/radio configuration and a lawful source/redistribution basis before any target build claim.

No Wi-Fi configuration, cloud credential, TLS key, course file, vendor binary, board action or private service is included in this Host foundation. Hardware and cloud tests require separate conditions and authorization.
