import socket
import os

socketToGm = "/tmp/gmESPSocket"
socketToControl = "/tmp/controlESPSocket" 

esp0Addr = "idk"
esp1Addr = "idk"
esp2Addr = "idk"
esp3Addr = "idk"
esp4Addr = "idk"
esp5Addr = "idk"

# removing these files if they exist, so we can recreate them
if(os.path.exists(socketToGm)):
    os.remove(socketToGm)

if(os.path.exists(socketToControl)):
    os.remove(socketToControl)

# bind with server
gmServer = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
gmServer.bind(socketToGm)

# bind with controller
controlServer = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
controlServer.bind(socketToControl)

# wait on server connect, then control connect
gmServer.listen(1)
print("ESp listening for game manager")

gmConn, _ = gmServer.accept()
print("gm accepted communication!")

controlServer.listen(1)
print("ESP listening for controller")

controlConn, _ = controlServer.accept()
print("controller accepted communication!")

# gm first sends number of players, store that
numPlayers = gmConn.recv(1)[0]
print("Number of players is gonna be " + str(numPlayers) + "!")

# create pipes between this main process and its children who conect to the actual esp boards
childRead, parentWrite = os.pipe()
parentRead, childWrite = os.pipe()

# create child for every player we will be having
for i in range(numPlayers):
    if(os.fork() == 0):
        # child runs this
        controlConn.close()
        gmConn.close()
        controlServer.close()
        gmServer.close()
        os.close(parentWrite)
        os.close(parentRead)

        print("we made a esp that will communicate with esp #" + str(i) + "!")

        os.close(childWrite)
        os.close(childRead)
        print("killing child")
        os._exit(0)
    
# parent runs this
os.close(childRead)
os.close(childWrite)

readyCheck = gmConn.recv(6)
readyCheck = readyCheck.decode()
if(readyCheck == "ready?"):
    gmConn.sendall(b"yes")
else:
    print("esp manager expected ready check, got something else!")
    gmConn.sendall(b"no")

print("Parent finished creating its beautiful ESP children!")
#stuff to do when finishing. first, wait for all children to die off like the pathetic saps they are
for _ in range(numPlayers):
    pid, status = os.wait()
    print("Child " + str(pid) + " returned with status " + str(status))
os.close(parentRead)
os.close(parentWrite)
controlConn.close()
gmConn.close()
controlServer.close()
gmServer.close()
os.remove(socketToGm)
os.remove(socketToControl)
print("killing parent")
