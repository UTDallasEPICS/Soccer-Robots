import socket
import os
from ESPClient import ESPClient

socketToGm = "/tmp/gmESPSocket"
socketToControl = "/tmp/controlESPSocket" 

espAddrs = {}
espAddrs["esp0"] = "10.42.0.201"
espAddrs["esp1"] = "idk"
espAddrs["esp2"] = "idk"
espAddrs["esp2"] = "idk"
espAddrs["esp3"] = "idk"
espAddrs["esp4"] = "idk"
espAddrs["esp5"] = "idk"

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
controlConn.settimeout(1)
print("controller accepted communication!")

# gm first sends number of players, store that
numPlayers = gmConn.recv(1)[0]
print("Number of players is gonna be " + str(numPlayers) + "!")
parentPipes = [[None, None] for _ in range(numPlayers)]

def getKeysFromNumbers(numInput):
    finalMessage = ""
    if(numInput[0] == "1"):
        finalMessage += "u"
    if(numInput[1] == "1"):
        finalMessage += "l"
    if(numInput[2] == "1"):
        finalMessage += "d"
    if(numInput[3] == "1"):
        finalMessage += "r"
    return finalMessage

# create pipes between this main process and its children who conect to the actual esp boards
# create child for every player we will be having
for i in range(numPlayers):
    childRead, parentWrite = os.pipe()
    parentRead, childWrite = os.pipe()
    if(os.fork() == 0):
        # child runs this
        del parentPipes
        os.close(parentWrite)
        os.close(parentRead)
        controlConn.close()
        gmConn.close()
        controlServer.close()
        gmServer.close()

        connection = ESPClient(espAddrs["esp" + str(i)], 30000)
        # 2 is the number of seconds to wait to connect
        espConnected = connection.tryConnect(2)
        print("we made a esp that will communicate with esp #" + str(i) + "!")

        # loop restarts at end of each match
        while(True):
            # first get check if ready
            readyCheck = os.read(childRead, 6)
            readyCheck = readyCheck.decode()
            if(readyCheck == "ready?"):
                # if disconnected, try to reconnection one more time
                if(not espConnected):
                    espConnected = connection.tryConnect(2)
                # if still disconnected, say it failed connection
                if(not espConnected):
                    print("Esp #" + str(i) + " has failed connection again! fraud")
                    os.write(childWrite, b"no")
                #else if connected that's nice. now, send request if its ready to the esp 
                else:
                    connection.send("readyCheck")
                    answer = connection.recv(2)
                    if(answer == "ready"):
                        print("Esp #" + str(i) + " is ready!")
                        os.write(childWrite, b"yes")
                    else:
                        print("esp #" + str(i) + " is not ready!")
                        os.write(childWrite, b"no")
            # otherwise we got something unintended, send that we ain't ready and restart the while loop
            else:
                print("Child here. Supposed to get ready command, but didn't! Instead got: " + readyCheck)
                os.write(childWrite, b"no")
            while(True):
                nextCommand = os.read(childRead, 10)
                nextCommand = nextCommand.decode()
                # if the parent wants us to reset, we gotta do that
                if(nextCommand == "reset"):
                    print("restarting at least once!")
                    break
                # else, we assume its a motor movement command
                else:
                    print("command is : " + nextCommand)
                    print("now getting input for an esp!")
                    # we don't really care yet about if it succeeded or not
                    formattedInput = getKeysFromNumbers(nextCommand)
                    connection.send(formattedInput)
            

        os.close(childWrite)
        os.close(childRead)
        print("killing child")
        os._exit(0)
    else:
        # store pipes of parent for that child
        parentPipes[i][0] = parentRead
        parentPipes[i][1] = parentWrite
        os.close(childRead)
        os.close(childWrite)
    
# parent runs this
print("Parent finished creating its beautiful ESP children!")

# loop restarts after each match
while(True):
    # wait for input
    readyCheck = gmConn.recv(6)
    readyCheck = readyCheck.decode()
    # if asking if ready, ask all the esps if they're ready
    if(readyCheck == "ready?"):
        # tell all the esp children that you have to check ready
        for i in range(numPlayers):
            os.write(parentPipes[i][1], readyCheck.encode())
        # initially set check to yes, if finding one that fails then make it "no"
        espReady = "yes"
        # now check all esp children what they return
        for i in range(numPlayers):
            askEsp = os.read(parentPipes[i][0], 3)
            askEsp = askEsp.decode()
            # if no, set entire result to no
            if(askEsp == "no"):
                print("one esp check failed!")
                espReady = "no"
        gmConn.sendall(espReady.encode())
        if(espReady == "no"):
            # now for each of the children, reset them back to their starting points
            for i in range(numPlayers):
                os.write(parentPipes[i][1], b"reset")
            continue
    else:
        print("esp manager expected ready check, got this instead: " + readyCheck)
        gmConn.sendall(b"error")
        continue
    # now, game starts!
    
    while(True):
        #here, briefly check the time

        #next, get controller data, and send it
        try:
            movementData = controlConn.recv(6)
            movementData = movementData.decode()
            # first one sent is the id
            playerId = int(movementData[0])
            # after the first two chars, that is the movement data
            movementData = movementData[2:]
            os.write(parentPipes[playerId][1], movementData.encode())
        # no big deal if we don't get data in that time
        except socket.timeout:
            print("")


#stuff to do when finishing. first, wait for all children to die off like the pathetic saps they are
for i in range(numPlayers):
    pid, status = os.wait()
    os.close(parentPipes[i][0])
    os.close(parentPipes[i][1])
    print("Child " + str(pid) + " returned with status " + str(status))
controlConn.close()
gmConn.close()
controlServer.close()
gmServer.close()
os.remove(socketToGm)
os.remove(socketToControl)
print("killing parent")
