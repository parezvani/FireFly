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
