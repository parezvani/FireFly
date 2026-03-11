import socket

ESP_IP = "192.168.4.1"
ESP_PORT = 11111

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print("Connected to ESP32 UDP test")
print("Press ENTER to send a packet (LED should toggle)")
print("Press q + ENTER to quit\n")

while True:
    cmd = input("> ")

    if cmd.lower() == "q":
        break

    message = "toggle"
    sock.sendto(message.encode(), (ESP_IP, ESP_PORT))
    print("Packet sent")

sock.close()
