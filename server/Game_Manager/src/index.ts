import express from "express"
import dotenv from "dotenv"
import cors from "cors"
import { WebSocket, WebSocketServer } from "ws"
import { createServer, IncomingMessage } from "http"
// import { createServer } from "https"
import jwt from "jsonwebtoken"
import fs from "fs"
import { PrismaClient } from "@prisma/client"
import { nanoid } from "nanoid"
import type {Player as PlayerType} from "@prisma/client" 


const prisma = new PrismaClient()

// Environment variables
dotenv.config({ path: "./.env" })
const PI_ADDR : string = process.env.PI_ADDR
const LOCALHOST: string = process.env.LOCALHOST ?? "localhost"
const PORT_SSE_GM: number = parseInt(`${process.env.PORT_SSE_GM}`)
const PORT_GM_RASPBERRY: number = parseInt(`${process.env.PORT_GM_RASPBERRY}`)
const PORT_CLIENT_GM: number = parseInt(`${process.env.PORT_CLIENT_GM}`)
const PORT_EXPRESS_CONTROLLER_GAMEMANAGER: number = parseInt(`${process.env.PORT_EXPRESS_CONTROLLER_GAMEMANAGER}`)

// SHARED VARIABLES
const queue: Array<{username: string, user_id: string, ws: any}> = []
const players: Array<{username: string, user_id: string, ws: any, accepted: boolean}> = []
let CONFIRMATION_PASSWORD: string = "sousounofrieren" // "Tearful goodbyes aren’t our style. It’d be embarrassing when we meet again"
let CONTROLLER_ACCESS: string = "donutvampire" // the initial value does not do anything here
let timer: number = 0
let timer_duration: number = parseInt(`${process.env.TIMER_DURATION}`) // this is the initial timer duration, in seconds
let confirmation_timer: number = 0 //tracks curent time that confirmation has been active.
const confirmation_timer_duration: number = parseInt(`${process.env.CONFIRMATION_TIMER_DURATION}`) // this is the time given to players to confirm, in seconds
let score1: number = 8
let score2: number = 5
enum GAME_STATE { NOT_PLAYING, SEND_CONFIRM, PLAYING, RESETTING }
let game_state: GAME_STATE = GAME_STATE.NOT_PLAYING

let robots_ready: boolean = true
let numPlayers: number = 1

// Match Settings
const matchSettings = async () => {
    const response = await prisma.matchSettings.findFirst({
        where: {id: 1}
    });
    //getting timer duration and numplayers from the matchsettings. if unable to find match length, set to default.
    timer_duration = response?.matchLength as unknown as number ? response?.matchLength as unknown as number : parseInt(`${process.env.TIMER_DURATION}`)
    numPlayers = response?.numPlayers as unknown as number

}

// SECTION: GAME CYCLES
//updates every second
const gameCycle = setInterval( async () => {
    if(game_state == GAME_STATE.NOT_PLAYING){
        // Check for sufficient users in queue to send confirmation request
        await matchSettings()
        if(queue.length >= 2){
            if(robots_ready){ // robots are ready to play
                game_state = GAME_STATE.SEND_CONFIRM
                CONFIRMATION_PASSWORD = nanoid() // new password for each confirmation attempt
                //to each client connected, send them a request to confirm the match. will be indices 0 and 1 as those are next up (will be
                //updated when there's more players)
                queue[0].ws.send(JSON.stringify({
                    "type": "MATCH_CONFIRMATION",
                    "payload": CONFIRMATION_PASSWORD
                }))
                queue[1].ws.send(JSON.stringify({
                    "type": "MATCH_CONFIRMATION",
                    "payload": CONFIRMATION_PASSWORD
                }))
                //set confirmation timer to begin
                confirmation_timer = confirmation_timer_duration
            }
            else{ // ask if robots are ready to play. If so it sends a message "IS_READY", handled at the end of the fil
                console.log("asking if ready")
                ws_raspberry.send(JSON.stringify({
                    "type": "CHECK_READY",
                    "payload": ""
                }))
            }
        }
    }
    //else if currently confirming
    else if(game_state == GAME_STATE.SEND_CONFIRM){
        // Check if received 2 confirmation response
        if(confirmation_timer == 0){ // time's up
            // 2 accepts -> start game
            if(players.length == 2 && players[0]["accepted"] && players[1]["accepted"]){
                game_state = GAME_STATE.PLAYING
                CONTROLLER_ACCESS = nanoid() // new access code for each game
                // tell Controller server to change access code, and sends the new one
                await fetch(`http://${LOCALHOST}:${PORT_EXPRESS_CONTROLLER_GAMEMANAGER}/accesspassword`, {
                    method: "POST",
                    headers: {"Content-Type": "application/json"},
                    body: JSON.stringify({
                        "accesspassword": CONTROLLER_ACCESS
                    })
                })
                
                //hardcode testing with players and sending to database//

                console.log(players[0]["username"] + " vs " + players[1]["username"])
                // authorize players in Controller server to send key inputs
                await fetch(`http://${LOCALHOST}:${PORT_EXPRESS_CONTROLLER_GAMEMANAGER}/addusers`, {
                    method: "POST",
                    headers: {"Content-Type": "application/json"},
                    //sending the users so controller knows which ones
                    body: JSON.stringify({ "users": [  {"user_id": players[0]["user_id"], "playernumber": 0}, 
                                            {"user_id": players[1]["user_id"], "playernumber": 1}] })
                })
                // give players the access code to connect to Controller server WebSocket
                queue[0].ws.send(JSON.stringify({
                    "type": "MATCH_START",
                    "payload": CONTROLLER_ACCESS
                }))
                queue[1].ws.send(JSON.stringify({
                    "type": "MATCH_START",
                    "payload": CONTROLLER_ACCESS
                }))
                // close ws because players are not in queue anymore
                queue[0]["ws"].close()
                queue[1]["ws"].close()
                //take them outta the queue array
                queue.splice(0, 2)
                //begin timer
                timer = timer_duration
                // tell Raspberry server to start the game
                ws_raspberry.send(JSON.stringify({
                    "type": "GAME_START",
                    "payload": {"timer": timer_duration}
                }))
            }
            else{ // did not get 2 accepts
                // signal players to reset confirmation 
                queue[0]["ws"].send(JSON.stringify({
                    "type": "MATCH_CONFIRMATION_RESET",
                    "payload": ""
                }))
                queue[1]["ws"].send(JSON.stringify({
                    "type": "MATCH_CONFIRMATION_RESET",
                    "payload": ""
                }))
                // find the player(s) that declined/did not respond and remove from queue/close ws connection
                // index of user queue[0] in players array
                const indexA: number = players.findIndex((element) => { return element["username"] === queue[0]["username"]})
                // index of user queue[1] in players array
                const indexB: number = players.findIndex((element) => { return element["username"] === queue[1]["username"]})
                let removeA: boolean = false
                let removeB: boolean = false
                if(indexA == -1 || players[indexA]["accepted"] === false){ // close connection for player A if did not respond or declined
                    queue[0]["ws"].close()
                    removeA = true
                }
                if(indexB == -1 || players[indexB]["accepted"] === false){ // close connection for player B if did not respond or declined
                    queue[1]["ws"].close()
                    removeB = true
                }
                // remove declined/did not respond players from queue
                if(removeA && removeB){
                    queue.splice(0, 2)
                }
                else if(removeA){
                    queue.splice(0, 1)
                }
                else if(removeB){
                    queue.splice(1, 1)
                }
                game_state = GAME_STATE.NOT_PLAYING
                players.splice(0, players.length) // clear array of players
            }
        }
        //each loop, the confirmation timer goes down
        confirmation_timer--
    }
    else if(game_state == GAME_STATE.PLAYING){
        // Check when timer reaches 0
        console.log(`TIMER: ${timer} | ${players[0]["username"]} vs ${players[1]["username"]}`)
        //timer--;
        if(timer == 0){
            //when timer over, go to reset state
            game_state = GAME_STATE.RESETTING
        }
    }
    else if(game_state == GAME_STATE.RESETTING){
        // Game end: remove players from authorization in Controller server and clear player array
        await fetch(`http://${LOCALHOST}:${PORT_EXPRESS_CONTROLLER_GAMEMANAGER}/removeusers`, {
            method: "POST",
            headers: {"Content-Type": "application/json"},
            //sending both users and their user id
            body: JSON.stringify({ "users": [  {"user_id": players[0]["user_id"]}, 
                                    {"user_id": players[1]["user_id"]}] })
        })


        

        // store played match in database
        await prisma.match.create({
            data: {
                //date of match is right now
                datetime: new Date(),


                // these are the players who played and their scores
                players: {
                    create: [
                        {
                            playerID: players[0]["user_id"],
                            playerScore: score1
                            
                        },
                        {
                            playerID: players[1]["user_id"],
                            playerScore: score2
                        }
                    ]


                }
            }
        })

        //finds player in the databases
       const [player1, player2]  = await prisma.$transaction([
        prisma.player.findFirst({
            where: {user_id: players[0]["user_id"]}
        }),
        prisma.player.findFirst({
            where: {user_id: players[1]["user_id"]}
        })
        ]) 

        // changes database values based on which player wins
        if(score1 > score2){
            //update W/L ratio of player 1 by increasing their wins if they won.
            //wif they have no losses, then we'd be dividing by 0, so here we avoid that. In such a case,
            //we just set it to their # of wins.
            let ratio1 = (player1 as PlayerType).losses ? ((player1 as PlayerType).wins + 1) / ((player1 as PlayerType).losses) : ++(player1 as PlayerType).wins

            await prisma.$transaction([
                prisma.player.update({
                    where: {user_id: players[0]["user_id"]},
                    data: {
                        //obvious stuff
                        wins: {increment: 1},
                        games: {increment: 1},
                        ratio: ratio1,
                        goals: {increment: score1}
                    }    
                }),
                
                //now for user two, add loss and update ratio
                prisma.player.update({
                    where: {user_id: players[1]["user_id"]},
                    data: {
                        losses: {increment: 1},
                        games: {increment: 1},
                        ratio: ((player2 as PlayerType).wins) / ((player2 as PlayerType).losses + 1),
                        goals: {increment: score2}

                    }
                    
                })
            ])
        }
        //if player two won
        else if(score2 > score1){
            //same idea as above, don't divide by 0
            let ratio2 = (player2 as PlayerType).losses ? ((player2 as PlayerType).wins + 1) / ((player2 as PlayerType).losses) : ++(player2 as PlayerType).wins
            await prisma.$transaction([
                prisma.player.update({
                    where: {user_id: players[0]["user_id"]},
                    data: {
                        losses: {increment: 1},
                        games: {increment: 1},
                        ratio: ((player1 as PlayerType).wins) / ((player1 as PlayerType).losses + 1),
                        goals: {increment: score1}

                    }
                    
                }),

                prisma.player.update({
                    where: {user_id: players[1]["user_id"]},
                    data: {
                        wins: {increment: 1},
                        games: {increment: 1},
                        ratio: ratio2,
                        goals: {increment: score2}

                    }
                    
                })
            ])
        }
        //else if they both tied. NOTE: PROBABLY WANT TO ADD # OF TIES IN GAMES
        else{
            await prisma.$transaction([
                prisma.player.update({
                    where: {user_id: players[0]["user_id"]},
                    data: {
                        //just increment games and goals, since they neither won nor lost
                        games: {increment: 1},
                        goals: {increment: score1}
                    }
                }),

                prisma.player.update({
                    where: {user_id: players[1]["user_id"]},
                    data: {
                        games: {increment: 1},
                        goals: {increment: score2}
                    }
                })
            ])
            
        }
        //remove them from the player queue
        players.splice(0, 2)
        //now that game just ended robots are not ready until the raspberry pi says so.
        robots_ready = false
        timer = 0
        //no idea why we do this, have to investigate
        score1 = score1 - 3
        score2 = score2
        game_state = GAME_STATE.NOT_PLAYING
    }
}, 1000)

// SECTION: WEBSOCKET LOGGED IN CLIENT <-> GAME MANAGER
const server_wss_CLIENT_GM = createServer()
const wss_client_gm = new WebSocketServer({ noServer: true })

//when a client connects to the game manager, do this
wss_client_gm.on("connection", (ws: any, request: IncomingMessage, username: string, user_id: string) => {
    console.log("New connection!")

    // kick user if ws connection is lost or closed
    ws.onclose = (event: any) => {
        //find their index in the queue, and remove them from it
        const index = queue.findIndex((element) => { return element["username"] === username })
        if(index != -1){
            console.log("REMOVING " + queue[index]["username"])
            queue.splice(index, 1)
        }
    }
    //when client sends message to game manager
    ws.on("message", (data: any) => {
        const { type, payload } = JSON.parse(data)
        console.log(`Received message => ${type} : ${payload}`)
        //when client wants to join queue. technically should already be in, to create the connection have to be in the queue
        if(type === "JOIN_QUEUE"){ // should already be in
            const index = queue.findIndex((element) => { return element.username === username })
            // do not let in if user is in game
            const player_index = players.findIndex((element) => { return element.username === username })
            //if somehow they're not in, add them to the queue
            if(index == -1 && player_index == -1){
                console.log("ADDING " + username)
                queue.push({"username": username, "user_id": user_id, "ws": ws})
            }
        }
        //when user wants to leave queue
        else if(type === "LEAVE_QUEUE"){
            //find their index in the queue
            const index = queue.findIndex((element) => { return element["username"] === username })
            //can remove two ways: if they were found, OR if they are not being sent a confirmation message
            //(those at indices 0 and 1 are sent confirm messages when game is in that state. Don't want the
            //user to be able to leave the queue without rejecting or accepting)
            if(index != -1 && !(game_state === GAME_STATE.SEND_CONFIRM && (index == 0 || index == 1))){
                console.log("REMOVING " + queue[index]["username"])
                queue.splice(index, 1)
            }
            //remove connection now that they've left
            ws.close()
        }
        //when user is sending confirmation that they're ready
        else if(type === "CONFIRMATION"){
            const { password, accepted } : { password: string, accepted: boolean } = payload
            //verify security that the confirmation password the client is sending is that which was given to them.
            if(password === CONFIRMATION_PASSWORD && game_state === GAME_STATE.SEND_CONFIRM){
                //see if the user is marked as a player now
                const player_index = players.findIndex((element) => { return element.username === username })
                // make sure players do not accept/decline multiple times
                if(player_index == -1){
                    // make sure users are the next 2 in queue
                    if(queue[0]["username"] === username || queue[1]["username"] === username){
                        //if so, add them to the player queue, even if they rejected
                        players.push({"username": username, "user_id": user_id, "ws": ws, "accepted": accepted})
                        console.log(`Player ${username} has ${accepted ? "accepted" : "declined"}`)
                    }
                }
            }
        }
    })
    ws.send("CONNECTED")
})

wss_client_gm.on("error", (error) => {
    console.log("WSS_CLIENT_GM error: " + error)
})

wss_client_gm.on("close", () => {
    console.log("WSS_CLIENT_GM closed")
})

// check for upgrade request to websocket from a logged in user
server_wss_CLIENT_GM.on("upgrade", async (request, socket, head) => {
    // Get cookies
    const cookies = request.headers["cookie"] ?? ""
    if(cookies === ""){ // if no cookies, close connection
        socket.destroy()
        return
    }
    //get the cookie pairs in an array, where an element in cookiepairs would be "key=value"
    const cookiepairs = cookies.split(";");
    //now each element in the cookiesplittedPairs array is another array with format [key, value]
    const cookiesplittedPairs = cookiepairs.map(cookie => cookie.split("="));
    const cookieObj: { [key: string]: string } = {}
    //for each key value pair, decode the cookie and turn the 2D array into a dictionary of keys and values
    cookiesplittedPairs.forEach((pair) => {
        // set each cookie value in the cookieObj
        cookieObj[decodeURIComponent(pair[0].trim())] = decodeURIComponent(pair[1].trim())
    })

    if(!cookieObj["srtoken"]){ // if no srtoken cookie, close connection
        socket.destroy()
        return
    }

    // Authenticate using jwt from cookie srtoken
    const srtoken = cookieObj["srtoken"]
    const claims: any = jwt.verify(srtoken, fs.readFileSync(process.cwd()+"/cert-dev.pem"), (error, decoded) => {
        //if error, close connection, otherwise return the decoded token
        if(error){ 
            socket.destroy()
            return
        }
        return decoded
    })

    if(!(claims instanceof Object && claims["sub"])){ // if jwt is invalid, close connection
        socket.destroy()
        return
    }

    //else, get user id from the token
    const user_id: string = claims["sub"]
    //make sure user is actually in the database already
    const find_user = await prisma.player.findUnique({
        where: {
            user_id: user_id
        }
    })
    if(!find_user){ // if user is not in database, close connection
        socket.destroy()
        return
    }

    // valid logged in user, upgrade connection to websocket
    wss_client_gm.handleUpgrade(request, socket, head, (ws) => {
        wss_client_gm.emit("connection", ws, request, find_user.username, user_id)
    })
})

server_wss_CLIENT_GM.listen(PORT_CLIENT_GM, () => {
    console.log(`SERVER_WSS_CLIENT_GM is running on http://${LOCALHOST}:${PORT_CLIENT_GM}`)
})

// SECTION: SERVER SENT EVENTS
const app_sse = express()
app_sse.use(cors())

const sse_clients: Array<any> = []

app_sse.listen(PORT_SSE_GM, () => {
    console.log(`SSE is running on http://${LOCALHOST}:${PORT_SSE_GM}`)
})

app_sse.get("/", (request, response) => {
    response.send("SSE SERVER")
})

// Establish SSE connection. This one broadcasts to all clients regardless of if they're in the queue or not.
app_sse.get("/sse-info", (request, response) => {
    const headers = {
        "Content-Type": "text/event-stream",
        "Connection": "keep-alive",
        "Cache-Control": "no-cache"
    }
    response.writeHead(200, headers)
    //sets the new client's id to current date
    const clientID = Date.now()
    const newClient = {
        id: clientID,
        response: response
    }
    console.log("sse-info: " + clientID)
    //adds client to the array
    sse_clients.push(newClient)

    request.on("close", () => {
        console.log("Client Connection closed")
    })
})

// Broadcast Queue, Timer, and Score1/Score2

//sets a loop running every second, which broadcasts the current queue to all the clients
const broadcastQueue = setInterval(() => {
    const queue_users: Array<string> = []
    //for each user the queue, add their username to the array that will be returned, queue_users
    queue.forEach((user) => {
        queue_users.push(user["username"])
    })
    //JSON-ify what will be sent, that being the updated queue with the appropriate payload
    const queue_update = JSON.stringify({"type": "UPDATE_QUEUE", "payload": queue_users})
    //for each client, write the response as the new queue
    sse_clients.forEach((client) => {
        client["response"].write("data: " + queue_update +"\n\n")
    })
}, 1000)

//sets a loop running every second, broacsasting the current timer to all clients, regarldess of if they're in the queue
const broadcastTimer = setInterval(() => {
    //JSON-ifie's the message type as updating the timer for the clients, and the payload being the value
    const timer_update = JSON.stringify({"type": "UPDATE_TIMER", "payload": timer})
    //for each client, write response as new timer
    sse_clients.forEach((client) => {
        client["response"].write("data: " + timer_update +"\n\n")
    })
}, 1000)


//sets a loop running every second, broacasting the scores of both players currently.
const broadcastScore = setInterval(() => {
    //set message type as updating the score, and give username and score of both players (needs to be changed for multiple players)
    const score_update = JSON.stringify({"type": "UPDATE_SCORE", 
                                        "payload": {"player1": {"username": players[0]?.["username"] ?? "", "score": score1},
                                                    "player2": {"username": players[1]?.["username"] ?? "", "score": score2 }}})
    //for each client, write response as new score for both users
    sse_clients.forEach((client) => {
        client["response"].write("data: " + score_update +"\n\n")
    })
}, 1000)

// SECTION: WEBSOCKET GAME MANAGER <-> RASPBERRY
// Make sure to set up Raspberry server first
const ws_raspberry = new WebSocket(`ws://${PI_ADDR}:${PORT_GM_RASPBERRY}`)

ws_raspberry.onopen = (event) => {
    console.log(`WS_RASPBERRY CONNECTED ws://${PI_ADDR}:${PORT_GM_RASPBERRY}`)
}

ws_raspberry.onerror = (error) => {
    console.log("WS_RASPBERRY error: " + error)
}
ws_raspberry.onclose = (event) => {
    console.log("WS_RASPBERRY closed")
}

//when the reaspberry pi sends a message to the game manager
ws_raspberry.onmessage = (event) => {
    const { type, payload } = JSON.parse(event.data.toString())
    // console.log(`Received message => ${type} : ${payload}`)

    //if the message is that the robots are ready, set robots_ready = true
    if(type === "IS_READY") {
        robots_ready = payload
        console.log(`Received message => ${type} : ${payload}`)
    }
    //if it's updating the timer, update that timer
    else if(type === "TIMER_UPDATE"){
        const { timer:timerUpdate } : { timer: number } = payload
        timer = timerUpdate
        console.log(`Received message => ${type} : ${timerUpdate}`)
    }
    //else if a player scores, update that
    else if(type === "SCORE_UPDATE"){
        const { score1:s1Update, score2:s2Update } : { score1: number, score2: number } = payload
        score1 = s1Update
        score2 = s2Update
        console.log(`Received message => ${type} : ${score1} ${score2}`)
    }
    //else if the game is ove, get scores and final time at the end, and save it
    else if(type === "GAME_END"){
        const { timer:finalTimer, score1:s1final, score2:s2final } : { timer: number, score1: number, score2: number } = payload
        // "payload": {"timer": 0, "score1": 0, "score2": 0}
        timer = finalTimer
        score1 = s1final
        score2 = s2final
        game_state = GAME_STATE.RESETTING
    }
}
