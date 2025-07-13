import asyncio
import websockets
import json
import time
import socket
#from NewServer import getTime()

# If packet is published properly print to screen
def on_publish(client, userdata, mid):
    print("Published to esp32")

# HOST = 'localhost'
# HOST = '192.168.250.96'
# HOST = '10.42.0.1' # Raspberry Pi self-hotspot
# HOST = '172.20.10.5'
# HOST = ''
# HOST = '10.158.225.101'
HOST = '192.168.177.101'
PORT = 1235

controlSocketPath = "/tmp/controlESPSocket"

controlSocket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
controlSocket.connect(controlSocketPath)
print("Connected to socket!")

# first element is for player 0, 2nd element is for play 1
prevData = ["0000", "0000"]

async def serverCM(websocket):
    print("inside CM")

    print("connected to esp!")

    # continue to receive data
    while True:
        received_data = await websocket.recv()
        received = json.loads(received_data)
        if received["type"] == "KEY_INPUT":
            # first get player number
            playerNum = received["payload"]["playernumber"]
            # if the data is same as before, dont' send it to EspManager. If it's different, then send it.
            if(prevData[playerNum] != received["payload"]["keys"]):
                print("Player " + str(playerNum) + " Input (bits): " + received["payload"]["keys"])
                # use "|" to differnetiate between player index and their input
                sentData = str(playerNum) + "|" + received["payload"]["keys"]
                controlSocket.sendall(sentData.encode())
                # sent current data to previous data now
                prevData[playerNum] = received["payload"]["keys"]
async def main():
    print("\nSTARTED CM SERVER")
    async with websockets.serve(serverCM, HOST, PORT):
        await asyncio.Future()

asyncio.run(main())
