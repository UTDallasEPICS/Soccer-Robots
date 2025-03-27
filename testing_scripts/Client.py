import socket

class Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Client called\r")

    def initialize(self):
        try:
            self.sock.connect((self.host, self.port))
            print(f"Connected to {self.host}:{self.port}")
        except socket.error:
            print(f"Error connecting to {self.host}:{self.port}")
            return

    def send(self, msg):
        try:
            self.sock.send(bytes(msg, 'utf-8'))
        except socket.error:
            return

