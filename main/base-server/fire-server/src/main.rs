use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};
use std::str::FromStr;
use std::thread;

#[derive(Debug)]
struct FireTelemetry {
    fire_detected: bool,
    fire_distance_m: f32,
    fire_range_m: f32,
    temperature_c: f32,
    humidity_pct: f32,
}

fn parse_request_line(line: &str) -> Result<(), String> {
    // Expect: METHOD PATH VERSION, e.g. "FIRE /telemetry DRONE/1.0"
    let parts: Vec<&str> = line.trim_end().split_whitespace().collect();
    if parts.len() != 3 {
        return Err("invalid request line".into());
    }
    let method = parts[0];
    let path = parts[1];
    let version = parts[2];

    if method != "FIRE" {
        return Err("unsupported method".into());
    }
    if path != "/telemetry" {
        return Err("unsupported path".into());
    }
    if !version.starts_with("DRONE/") {
        return Err("unsupported version".into());
    }
    Ok(())
}

fn parse_telemetry_line(line: &str) -> Result<FireTelemetry, String> {
    // Example:
    // "Fire-Detected: yes; Fire-Distance: 120; Fire-Range: 45; Temperature: 63.4; Humidity: 21.5"
    let mut fire_detected = None;
    let mut fire_distance_m = None;
    let mut fire_range_m = None;
    let mut temperature_c = None;
    let mut humidity_pct = None;

    for part in line.trim_end().split(';') {
        let kv = part.trim();
        if kv.is_empty() {
            continue;
        }
        let mut iter = kv.splitn(2, ':');
        let key = iter.next().ok_or("missing key")?.trim();
        let value = iter.next().ok_or("missing value")?.trim();

        match key {
            "Fire-Detected" => {
                fire_detected = Some(match value {
                    "yes" | "YES" | "Yes" => true,
                    "no" | "NO" | "No" => false,
                    _ => return Err("invalid Fire-Detected value".into()),
                });
            }
            "Fire-Distance" => {
                fire_distance_m = Some(f32::from_str(value).map_err(|_| "invalid Fire-Distance")?);
            }
            "Fire-Range" => {
                fire_range_m = Some(f32::from_str(value).map_err(|_| "invalid Fire-Range")?);
            }
            "Temperature" => {
                temperature_c = Some(f32::from_str(value).map_err(|_| "invalid Temperature")?);
            }
            "Humidity" => {
                humidity_pct = Some(f32::from_str(value).map_err(|_| "invalid Humidity")?);
            }
            _ => {
                // Ignore unknown keys or return Err if you want strictness
            }
        }
    }

    Ok(FireTelemetry {
        fire_detected: fire_detected.ok_or("missing Fire-Detected")?,
        fire_distance_m: fire_distance_m.ok_or("missing Fire-Distance")?,
        fire_range_m: fire_range_m.ok_or("missing Fire-Range")?,
        temperature_c: temperature_c.ok_or("missing Temperature")?,
        humidity_pct: humidity_pct.ok_or("missing Humidity")?,
    })
}

fn handle_client(mut stream: TcpStream) {
    let mut buf = [0u8; 1024];
    let n = match stream.read(&mut buf) {
        Ok(0) => return,
        Ok(n) => n,
        Err(_) => return,
    };

    let received = match std::str::from_utf8(&buf[..n]) {
        Ok(s) => s,
        Err(_) => {
            let _ = stream.write_all(b"DRONE/1.0 400 FAIL\r\n");
            return;
        }
    };

    let mut lines = received.lines();
    let request_line = match lines.next() {
        Some(l) if !l.is_empty() => l,
        _ => {
            let _ = stream.write_all(b"DRONE/1.0 400 FAIL\r\n");
            return;
        }
    };

    if let Err(e) = parse_request_line(request_line) {
        eprintln!("Bad request line: {e}");
        let _ = stream.write_all(b"DRONE/1.0 400 FAIL\r\n");
        return;
    }

    let telemetry_line = match lines.next() {
        Some(l) if !l.is_empty() => l,
        _ => {
            let _ = stream.write_all(b"DRONE/1.0 400 FAIL\r\n");
            return;
        }
    };

    match parse_telemetry_line(telemetry_line) {
        Ok(t) => {
            println!("Fire Detected: {}", if t.fire_detected { "yes" } else { "no" });
            println!("Fire Distance: {:.2} meters", t.fire_distance_m);
            println!("Fire Range: {:.2} meters", t.fire_range_m);
            println!("Temperature: {:.2} C", t.temperature_c);
            println!("Humidity: {:.2} %", t.humidity_pct);

            let _ = stream.write_all(b"DRONE/1.0 200 OK\r\n");
        }
        Err(e) => {
            eprintln!("Parse error: {e}");
            let _ = stream.write_all(b"DRONE/1.0 400 FAIL\r\n");
        }
    }
}

fn main() -> std::io::Result<()> {
    let listener = TcpListener::bind("0.0.0.0:7878")?;
    println!("Listening on 0.0.0.0:7878...");

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                thread::spawn(|| handle_client(stream));
            }
            Err(e) => eprintln!("Connection failed: {e}"),
        }
    }
    Ok(())
}
