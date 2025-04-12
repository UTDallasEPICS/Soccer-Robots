import socket
import os

ip = "10.42.0.1"
socketToGm = "/tmp/gmESPSocket"
socketToControl = "/tmp/controlESPSocket" 

esp1Addr = "idk"

# removing these files if they exist, so we can recreate them
if(os.path.exists(socketToGm)):
    os.remove(socketToGm)

if(os.path.exists(socketToControl)):
    os.remove(socketToControl)

gmServer = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
gmServer.bind(socketToGm)
gmServer.listen(1)
print("ESp listening for game manager")

conn, _ = gmServer.accept()
print("gm accepted communication!")

controlServer = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
controlServer.bind(socketToControl)
controlServer.listen(1)
print("ESP listening for controller")
conn, _ = controlServer.accept()
print("controller accepted communication!")

numPlayers = gmServer.recv(1)[0]
print("Number of players is gonna be " + str(numPlayers) + "!")
