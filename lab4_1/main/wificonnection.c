#include <WiFi.h>
#include <AsyncUDP.h>

const char ssid = "ESP-DRONE_DEMO";
const charpassword = "12345678";

// LED pin (adjust to your board)
const int LED_PIN = 2; // Built-in LED on many ESP32 boards

AsyncUDP udp;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Start Wi-Fi in Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("Wi-Fi AP started:");
  Serial.print("  SSID: "); Serial.println(ssid);
  Serial.print("  Pass: "); Serial.println(password);

  // Start UDP listening on port 9000 (common in ESP-Drone docs)
  if(udp.listen(9000)) {
    Serial.println("UDP listener started on port 9000.");

    udp.onPacket([](AsyncUDPPacket packet) {
      Serial.print("UDP Packet received: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();

      // Turn on LED to show connection succeeded
      digitalWrite(LED_PIN, HIGH);
    });

  } else {
    Serial.println("Failed to start UDP listener!");
  }
}

void loop() {
  // Nothing needed here — UDP callback handles events
}