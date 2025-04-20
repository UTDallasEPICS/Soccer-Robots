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
HOST = '192.168.134.90'
PORT = 1235

controlSocketPath = "/tmp/controlESPSocket"

async def serverCM(websocket, path):
    print("inside CM")

    controlSocket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    controlSocket.connect(controlSocketPath)
    print("connected to esp!")

    while True:
        received_data = await websocket.recv()
        received = json.loads(received_data)
        if received["type"] == "KEY_INPUT":
            if received["payload"]["playernumber"] == 0:
                if received["payload"]["keys"] != "0000":
                    # currInput = int(received["payload"]["keys"], 2)
                    # print("Player One Input (decimal): " + str(currInput))
                    print("Player One Input (bits): " + received["payload"]["keys"])
            if received["payload"]["playernumber"] == 1:
                if received["payload"]["keys"] != "0000":
                    # currInput = int(received["payload"]["keys"], 2)
                    # print("Player Two Input (decimal): " + str(currInput))
                    print("Player Two Input (bits): " + received["payload"]["keys"])
            sentData = str(received["payload"]["playernumber"]) + "|" + received["payload"]["keys"]
            controlSocket.sendall(sentData.encode())


start_server = websockets.serve(serverCM, HOST, PORT)
print("\nSTARTED CM SERVER")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
