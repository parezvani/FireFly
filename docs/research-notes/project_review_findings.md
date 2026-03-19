# Project Review Findings

Date: 2026-03-18

Scope reviewed:
- Active embedded firmware in `main/scripts/**` and `main/testing/motor/**`
- Base-station and telemetry code in `main/base-server/**`
- Build and configuration surfaces at the repo root and in reference projects

## Findings

### Critical

1. The motor-control path accepts unauthenticated and unbounded ESP-NOW commands.
   - `main/scripts/ESPNowEasy.cpp:75` accepts any packet with `len >= sizeof(MessageType)`.
   - `main/scripts/MotorReceiver.cpp:96` ignores `src_mac` instead of validating it against `remote_mac`.
   - `main/scripts/MotorReceiver.cpp:101` through `main/scripts/MotorReceiver.cpp:116` compute and apply PWM duty without clamping.
   - `main/testing/motor/main/motor.c:105` through `main/testing/motor/main/motor.c:107` already show the safer clamped behavior in the test firmware.
   - Impact: any same-channel ESP-NOW sender can inject a 4-byte payload and command the motor; malformed values can push invalid LEDC duty values.

### High

2. The repo-root ESP-IDF build does not include the actual drone receiver or motor firmware.
   - `main/CMakeLists.txt:1` only registers `scripts/RemoteSender.cpp` and `scripts/ESPNowEasy.cpp`.
   - `main/scripts/MotorReceiver.cpp:120` contains a separate `app_main`, but it is not part of the root build.
   - Impact: the default firmware build path produces the controller-side prototype only, not the motor-side receiver.

3. The ESP-NOW bridge and the Rust ground-station server are disconnected.
   - `main/base-server/espnow_drone_base_server/main/fire_bridge.c:20` only receives packets and pretty-prints them to UART.
   - `main/base-server/espnow_drone_base_server/main/fire_bridge.c:106` starts the bridge loop with no forwarding behavior.
   - `main/base-server/fire-server/src/main.rs:181` starts a separate TCP server on `0.0.0.0:7878`.
   - Impact: the in-repo drone -> bridge -> server telemetry path is not implemented end-to-end.

4. The Rust server assumes a complete request arrives in a single TCP `read()`.
   - `main/base-server/fire-server/src/main.rs:117` through `main/base-server/fire-server/src/main.rs:125` read once into a fixed buffer.
   - `main/base-server/fire-server/src/main.rs:150` immediately treats the second parsed line as complete telemetry.
   - Impact: valid fragmented TCP requests can be rejected with `400 FAIL` or parsed as partial data.

5. The Rust server creates an unbounded blocking thread per connection with no timeout.
   - `main/base-server/fire-server/src/main.rs:119` blocks on `stream.read()`.
   - `main/base-server/fire-server/src/main.rs:185` through `main/base-server/fire-server/src/main.rs:188` spawn a new thread for every incoming connection.
   - Impact: idle or abusive clients can pin threads indefinitely and exhaust server resources.

6. The Rust project files are lowercase, which breaks Cargo on case-sensitive filesystems.
   - `main/base-server/fire-server/cargo.toml:1` should be `Cargo.toml`.
   - `main/base-server/fire-server/cargo.lock:1` should be `Cargo.lock`.
   - `main/base-server/README.md:6` through `main/base-server/README.md:8` instruct users to run Cargo there.
   - Impact: this may appear fine on a default macOS checkout, then fail on Linux or any case-sensitive filesystem.

7. The top-level ESP-IDF project hard-requires `IDF_PATH` even though the repo README claims there is a local ESP-IDF checkout.
   - `CMakeLists.txt:3` includes `project.cmake` only from `$ENV{IDF_PATH}`.
   - `README.md:26` says the repo contains `esp/esp-idf/`.
   - Impact: a fresh checkout with only the documented local tree still will not configure unless `IDF_PATH` is manually exported.

8. The ESP-NOW base-station link is fragile because it depends on hard-coded board identity and implicit channel selection.
   - `main/base-server/espnow_drone_sender/main/fire_drone.c:18` hard-codes the bridge MAC address.
   - `main/base-server/espnow_drone_sender/main/fire_drone.c:153` through `main/base-server/espnow_drone_sender/main/fire_drone.c:156` add the peer with `channel = 0`.
   - `main/base-server/espnow_drone_base_server/main/fire_bridge.c:91` starts Wi-Fi but does not pin an explicit channel.
   - Impact: swapping boards or drifting off the implicit channel can silently break delivery with no recovery flow.

### Medium

9. `DroneReceive.cpp` is out of sync with the active ESP-NOW wrapper and will not compile if it is added back to a build.
   - `main/scripts/ESPNowEasy.h:14` defines `ReceiverCallback` as `(MessageType&, const uint8_t*)`.
   - `main/scripts/DroneReceive.cpp:12` still defines `handleIncoming(Message&)`.
   - `main/scripts/DroneReceive.cpp:24` passes that outdated callback signature into `onReceive`.
   - Impact: the file is stale and masked only because the root build currently excludes it.

10. `fire_drone.c` ignores the SHTC3 CRC bytes.
   - `main/base-server/espnow_drone_sender/main/fire_drone.c:86` reads the full 6-byte SHTC3 frame.
   - `main/base-server/espnow_drone_sender/main/fire_drone.c:100` through `main/base-server/espnow_drone_sender/main/fire_drone.c:105` convert raw values without validating `data[2]` or `data[5]`.
   - Impact: I2C bus corruption can become believable but incorrect telemetry.

11. The repo contains a stale parallel `ESPNowEasy` implementation and reference project with no guardrails.
   - `main/scripts/ESPNowEasy.h:14` uses the active two-argument callback API.
   - `main/lab4_1/main/ESPNowEasy.h:15` still uses the older one-argument callback API.
   - `main/lab4_1/CMakeLists.txt:6` keeps that tree as a standalone ESP-IDF project.
   - `main/lab4_1/README.md:4` shows it is an unrelated stock I2C example.
   - Impact: it is easy to edit or build the wrong tree and follow misleading instructions.

## Review Notes

- This review was source-based and validated across multiple project areas.
