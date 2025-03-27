import asyncio
import websockets
import json
import time
#from NewServer import getTime()

# If packet is published properly print to screen
def on_publish(client, userdata, mid):
    print("Published to esp32")

#HOST = 'localhost'
HOST = '192.168.137.195'
PORT = 1235

async def serverCM(websocket, path):
	print("inside CM")

	while True:
		received_data = await websocket.recv()
		received = json.loads(received_data)
		if received["type"] == "KEY_INPUT":
			if received["payload"]["playernumber"] == 0:
				if received["payload"]["keys"] != "0000":
					print("Player One Moved")
					currInput = int(received["payload"]["keys"], 2)
					print(f"Player One Input: {currInput}")
			if received["payload"]["playernumber"] == 1:
				if received["payload"]["keys"] != "0000":
					print("Player Two Moved")
					currInput = int(received["payload"]["keys"], 2)
					print(f"Player Two Input: {currInput}")

start_server = websockets.serve(serverCM, HOST, PORT)
print("\nSTARTED CM SERVER")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
