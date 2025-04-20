import asyncio
import socket
import websockets
import json
import time
import os
import mmap
# from backupTag import runTrakcer

#HOST = 'localhost'
HOST = '192.168.134.90'
PORT = 1234

espSocketPath = "/tmp/gmESPSocket"

sharedMemory = "/tmp/shared_timer"

game_time = 0
num_players = 1

def getTime():
    return game_time
runBackTracking = False

async def serverGM(websocket, path):
    while True:
        game_time = 0
        team1Score = 0
        team2Score = 1
        isReady = False 

        print("inside GM")

        #before communicating with the esp, to prevent race conditions first allocate sharemd memory.
        timerFile = os.open(sharedMemory, os.O_CREAT | os.O_RDWR)
        os.ftruncate(timerFile, 1)
        memLocation = mmap.mmap(timerFile, 1)

        # connect with esp now
        espSocket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        espSocket.connect(espSocketPath)
        print("connected to esp manager!")
        espSocket.sendall(num_players.to_bytes(1, "little")) 

        while isReady==False:
            received_data = await websocket.recv()
            received = json.loads(received_data)
            print(received)
            if received["type"] =="CHECK_READY":
                # now, check with the esp manager if its ready
                espSocket.sendall(b"ready?")
                readyCheck = espSocket.recv(3)
                readyCheck = readyCheck.decode()
                # if we return that they are ready, send that to the website
                if(readyCheck == "yes"):
                    print("READY GRAHHHHH")
                    isReady = True
                    await websocket.send(json.dumps({
                        "type": "IS_READY",
                        "payload": True
                    }))
                    break
                else:
                    # else we'd have returned that they're not ready
                    print("Tried to check if ready, the ESP's are not!")
                    await websocket.send(json.dumps({
                        "type": "IS_READY",
                        "payload": False
                    }))
            else:
                print("Supposed to receive ready signal, what we actually got: " + received["type"])

        received_data = await websocket.recv()
        received = json.loads(received_data)    
        print(received["payload"], type(received["payload"]))
        game_time = received["payload"]["timer"]
        print("\nTimer:")
        
        score_update = {
            "type": "SCORE_UPDATE",
            "payload": {"score1": team1Score, "score2": team2Score}
        }

        while game_time >= 0 and isReady:
            if game_time == 0:
                final_score_update = {"type": "GAME_END","payload": {"timer": 0, "score1": team1Score, "score2": team2Score}}
                await websocket.send(json.dumps(final_score_update))
                isReady = False
                runBackTracking = True
                memLocation[:1] = bytes([0])
                break

            if game_time%7==0:
                team1Score += 1
            elif game_time%9==0:
                team2Score += 1

            print("Send Current Time: "+str(game_time))
            await websocket.send(json.dumps({"type": "SCORE_UPDATE", "payload": {"score1": team1Score, "score2": team2Score}}))
            await websocket.send(json.dumps({"type":"TIMER_UPDATE","payload":{"timer":game_time}}))
            time.sleep(1)
            game_time -= 1

        print("\n\n")
        print("FINAL SCORE: Team 1: ", team1Score, " Team 2: ", team2Score)

        # if runBackTracking:
            # test = runTrakcer()
            # runBackTracking = False

start_server = websockets.serve(serverGM, HOST, PORT)
print("STARTED GM SERVER")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
