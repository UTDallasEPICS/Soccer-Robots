import socket

class ESPClient:
    # initialize and prepare for the connection from the process on the Pi to a esp32
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connectStatus = False

    # try to connect to the esp 32.
    def tryConnect(self, timeoutTime):
        try:
            # set a timeout to connect
            self.sock.settimeout(timeoutTime)
            self.sock.connect((self.host, self.port))
            # just send garbage, want to just see if it succeeds in sending
            self.sock.sendall(b"ignore|")
            print(f"Connected to {self.host}:{self.port}")
            self.connectStatus = True
            return True
        # if timeout, say that's the reason why we timed out
        except socket.timeout:
            print(f"We timed out connecting to {self.host}:{self.port}")
            # since connect status failed, set it to false
            self.connectStatus = False
            return False
        except socket.error:
            print(f"Error connecting to {self.host}:{self.port}")
            self.connectStatus = False
            return False
        # reset timeout, as we may not want it
        finally:
            self.sock.settimeout(None)

    # this command sends data from the pi process to the esp
    def send(self, msg):
        try:
            # include the "|" as a delimeter to differentiate between different messages
            self.sock.sendall(bytes(msg + "|", 'utf-8'))
            return True
        # if socket send fails, set connect status to false
        except socket.error:
            self.connectStatus = False
            return False

    # this command receives data from the esp that's sent to the raspberry pi
    def recv(self, timeoutTime):
        # set timeout to receive
        self.sock.settimeout(5)            
        try:
            buffer = ""
            # we read one byte at a time cause we can't really receive everything at once, read until delimmeter
            while(True):
                data = self.sock.recv(1)
                if(data.decode() == '|'):
                    break
                buffer += data.decode()
            return buffer
        # again if error, set connect status to false since the connection failed
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
