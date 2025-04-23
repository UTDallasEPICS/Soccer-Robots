import socket

class ESPClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connectStatus = False

    def tryConnect(self, timeoutTime):
        try:
            self.sock.settimeout(timeoutTime)
            self.sock.connect((self.host, self.port))
            self.sock.sendall(b"ignore|")
            print(f"Connected to {self.host}:{self.port}")
            self.connectStatus = True
            return True
        except socket.timeout:
            print(f"We timed out connecting to {self.host}:{self.port}")
            self.connectStatus = False
            return False
        except socket.error:
            print(f"Error connecting to {self.host}:{self.port}")
            self.connectStatus = False
            return False
        finally:
            self.sock.settimeout(None)

    def send(self, msg):
        try:
            self.sock.sendall(bytes(msg + "|", 'utf-8'))
            return True
        except socket.error:
            self.connectStatus = False
            return False

    def recv(self, timeoutTime):
        self.sock.settimeout(5)            
        try:
            currentChar = 'Z'
            buffer = ""
            while(True):
                data = self.sock.recv(1)
                if(data.decode() == '|'):
                    break
                buffer += data.decode()
            return buffer
        except socket.timeout:
            print("esp receive timed out!")
            self.connectStatus = False
            return "timeout"
        except socket.error:
            print("some other error receiving!")
            self.connectStatus = False
            return "error"
        finally:
            self.sock.settimeout(None)
