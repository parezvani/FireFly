import socket

# Bind to all interfaces (0.0.0.0) and all UDP ports
UDP_IP = "0.0.0.0"
UDP_PORT = 0  # 0 lets OS pick, but we still receive all packets

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, 0))  # bind to all interfaces on a random port
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

print("Listening for any UDP packet on all ports... (Ctrl+C to stop)")

while True:
    try:
        data, addr = sock.recvfrom(1024)
        print(f"Packet from {addr[0]}:{addr[1]} -> {data}")
    except KeyboardInterrupt:
        print("\nExiting...")
        break
