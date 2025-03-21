import { WebSocketServer } from "ws"
import dotenv from "dotenv"

// Environment variables
dotenv.config({ path: "./.env" })
const LOCALHOST: string = process.env.LOCALHOST ?? "localhost"
const PORT_GM_RASPBERRY: number = parseInt(`${process.env.PORT_GM_RASPBERRY}`)
const PORT_WSS_CONTROLLER_RASPBERRY: number = parseInt(`${process.env.PORT_WSS_CONTROLLER_RASPBERRY}`)

// Shared variables
let ready: boolean = true
let timer: number = 0
let score1: number = 10
let score2: number = 2

// FOR GAME MANAGER
const wss_gm = new WebSocketServer({ port: PORT_GM_RASPBERRY})

wss_gm.on("listening", () => {
    console.log(`WebSocket wss_gm is running on ws://${LOCALHOST}:${PORT_GM_RASPBERRY}`)
})

wss_gm.on("error", (error) => {
    console.log("WebSocket wss_gm error: " + error)
})

wss_gm.on("close", () => {
    console.log("WebSocket wss_gm closed")
})

//when the game manager connects
wss_gm.on("connection", (ws: any, request) => {
    //if the game manager sends a message, do this
    ws.on("message", (data: any) => {
        const { type, payload } = JSON.parse(data)
        //if GM is wanting to check if ready, this just tells it for now that it's ready. SHOULD BE CHANGED TO ACTUALLY VERIFY FROM THE RASPBERRY
        if(type === "CHECK_READY"){ // should already be in
            console.log(`Telling GM: ready is ${ready}`)
            ws.send(JSON.stringify({
                "type": "IS_READY",
                "payload": ready
            }))
        }
        //if GM is wanting to start the game, do just that.
        else if(type === "GAME_START"){
            timer = payload["timer"]
            //every second, the broadcast timer updates, decrementing by 1
            const broadcastTimer = setInterval(() => {
                //when timer over
                if(timer === 0){
                    clearInterval(broadcastTimer)
                    clearInterval(broadcastScore)
                    //send message that game over to game manager
                    ws.send(JSON.stringify({
                        "type": "GAME_END",
                        "payload": {"timer": timer, "score1": score1, "score2": score2}
                    }))
                }
                else{
                    //else send new timer to game manager
                    ws.send(JSON.stringify({
                        "type": "TIMER_UPDATE",
                        "payload": {"timer": timer}
                    }))
                    timer--
                }
            }, 1000)
            //every second, sending the new score back to the game manager
            const broadcastScore = setInterval(() => {
                ws.send(JSON.stringify({
                    "type": "SCORE_UPDATE",
                    "payload": {"score1": score1, "score2": score2}
                }))
            }, 1000)
        }
    })
})

// FOR CONTROLLER
const wss_control = new WebSocketServer({ port: PORT_WSS_CONTROLLER_RASPBERRY})

wss_control.on("listening", () => {
    console.log(`WebSocket wss_control is running on ws://${LOCALHOST}:${PORT_WSS_CONTROLLER_RASPBERRY}`)
})

wss_control.on("error", (error) => {
    console.log("WebSocket wss_control error: " + error)
})

wss_control.on("close", () => {
    console.log("WebSocket wss_control closed")
})

//when controller connects with the rasbperry pi website server
wss_control.on("connection", (ws: any, request) => {
    //when the controller sends a message, curretnly just for key input
    ws.on("message", (data: any) => {
        const { type, payload } = JSON.parse(data)
        //if its key input, we get the keys pressed and player number values
        if(type === "KEY_INPUT"){ // should already be in
            const { keys, playernumber }: {keys: string, playernumber: number} = payload
            //right now, we don't send it to the raspberry pi
            console.log(`Player ${playernumber} pressed ${keys}`)
        }
    })
})

//NOTE: AS OF RIGHT NOW, the raspberry Pi is NOT connected to the website. We'll have to change this.