import asyncio
import websockets
import json
import time
from backupTag import runTrakcer

#HOST = 'localhost'
HOST = '10.42.0.1'
PORT = 1234

game_time = 0

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
        while isReady==False:
            received_data = await websocket.recv()
            received = json.loads(received_data)
            print(received)
            if received["type"] =="CHECK_READY":
                print("READY GRAHHHHH")
                isReady = True
                await websocket.send(json.dumps({
                    "type": "IS_READY",
                    "payload": True
                }))
                break
            else:
                print("Supposed to receive ready signal, what we actually got: " + received["type"])
                print("NOT READY")

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

        if runBackTracking:
            test = runTrakcer()
            runBackTracking = False

start_server = websockets.serve(serverGM, HOST, PORT)
print("STARTED GM SERVER")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
