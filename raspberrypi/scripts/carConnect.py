import socket
import os

def get_pi_ip():
  try:
    ip = os.popen('hostname -I').read().strip()
    return ip.split()[0]
  except Exception as e:
    print(f"Failed to get Pi IP: {e}")
    return None

PI_IP = get_pi_ip()

# Replace with ESP32 IP address and port
ESP32_IP = '192.168.1.100'
ESP32_PORT = 80

def connect_to_esp32(ip, port):
  try:
    # Create a socket object
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Connect to the ESP32
    s.connect((ip, port))
    print(f"Connected to ESP32 at {ip}:{port}")
    
    # Send a test message
    message = "Hello ESP32"
    s.sendall(message.encode())
    
    # Receive response
    response = s.recv(1024)
    print(f"Received from ESP32: {response.decode()}")
    
    # Close the connection
    s.close()
  except Exception as e:
    print(f"Failed to connect to ESP32: {e}")

if __name__ == "__main__":
  connect_to_esp32(ESP32_IP, ESP32_PORT)