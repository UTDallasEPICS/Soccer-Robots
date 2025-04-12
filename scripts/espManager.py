import socket
import os

socketToGm = "/tmp/gmESPSocket"
socketToControl = "/tmp/controlESPSocket" 

esp0Addr = "idk"

# removing these files if they exist, so we can recreate them
if(os.path.exists(socketToGm)):
    os.remove(socketToGm)

if(os.path.exists(socketToControl)):
    os.remove(socketToControl)

gmServer = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
gmServer.bind(socketToGm)

controlServer = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
controlServer.bind(socketToControl)

gmServer.listen(1)
print("ESp listening for game manager")

gmConn, _ = gmServer.accept()
print("gm accepted communication!")

controlServer.listen(1)
print("ESP listening for controller")

controlConn, _ = controlServer.accept()
print("controller accepted communication!")

numPlayers = gmConn.recv(1)[0]
print("Number of players is gonna be " + str(numPlayers) + "!")

for i in range(numPlayers):
    if(os.fork() == 0):
        # child runs this
        controlConn.close()
        gmConn.close()
        controlServer.close()
        gmServer.close()
        print("we made a esp that will communicate with esp #" + str(i) + "!")
        os._exit()
    
# parent runs this
print("Parent finished creating its beautiful ESP children!")
controlConn.close()
gmConn.close()
controlServer.close()
gmServer.close()
os.remove(socketToGm)
os.remove(socketToControl)
