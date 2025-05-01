import asyncio
import websockets
import json
import time
import socket
#from NewServer import getTime()

# If packet is published properly print to screen
def on_publish(client, userdata, mid):
    print("Published to esp32")

#HOST = 'localhost'
HOST = '192.168.250.90'
PORT = 1235

controlSocketPath = "/tmp/controlESPSocket"

controlSocket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
controlSocket.connect(controlSocketPath)

# first element is for player 0, 2nd element is for play 1
prevData = ["0000", "0000"]

async def serverCM(websocket, path):
    print("inside CM")

    print("connected to esp!")

    while True:
        received_data = await websocket.recv()
        received = json.loads(received_data)
        if received["type"] == "KEY_INPUT":
            playerNum = received["payload"]["playernumber"]
            if(prevData[playerNum] != received["payload"]["keys"]):
                print("Player " + str(playerNum) + " Input (bits): " + received["payload"]["keys"])
                sentData = str(playerNum) + "|" + received["payload"]["keys"]
                controlSocket.sendall(sentData.encode())
                prevData[playerNum] = received["payload"]["keys"]


start_server = websockets.serve(serverCM, HOST, PORT)
print("\nSTARTED CM SERVER")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
