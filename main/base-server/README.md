FIRE_SERVER:
------------------------------------------------------------------------------------------------------------------------------------------
Sends message using cargo/rust. 
If you want to test this out on your own you can download the "fire_server" directory. 

Open two instances of Ubuntu:
- On one run "cargo run --bin fire_server", 
- On the other run "cargo run --bin fire_server"

Send message from drone to server in the following format:
- Request Line: FIRE /telemetry DRONE/1.0\n
- Telemetry Line: Fire-Detected: yes; Fire-Distance: 120; Fire-Range: 45; Temperature: 63.4; Humidity: 21.5; Drone-ID: DRONE-01\n

Full Payload:
FIRE /telemetry DRONE/1.0\nFire-Detected: yes; Fire-Distance: 120; Fire-Range: 45; Temperature: 63.4; Humidity: 21.5; Drone-ID: DRONE-01\n

------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------




ESPNOW DIRECTORIES:
------------------------------------------------------------------------------------------------------------------------------------------
Sends messages between two ESP32c3-devkit-Rust-1 microcontrollers
Uses ESP-IDF

Build and flash fire_bridge on base station esp

Build and flash fire_drone on drone esp

Currently takes temperature and humidity readings from onboard drone ESP IMU and sends them to the base server ESP through ESP-NOW

Same payload construction as fire_server:
FIRE /telemetry DRONE/1.0\nFire-Detected: yes; Fire-Distance: 120; Fire-Range: 45; Temperature: 63.4; Humidity: 21.5; Drone-ID: DRONE-01\n



