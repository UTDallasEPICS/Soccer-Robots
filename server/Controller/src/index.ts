import express from "express"
import { PrismaClient } from '@prisma/client'   
import { WebSocket, WebSocketServer } from "ws"
import { createServer, IncomingMessage } from "http"
import jwt from "jsonwebtoken"
import fs from "fs"
import dotenv from "dotenv"
import cors from 'cors';


interface JwtPayloadWithRole extends jwt.JwtPayload {
    role?: string;
  }
  
// Environment variables
dotenv.config({ path: "./.env" })
const PI_ADDR : string = process.env.PI_ADDR
const LOCALHOST: string = process.env.LOCALHOST ?? "localhost"
const PORT_SERVER: number = parseInt(`${process.env.PORT_EXPRESS_CONTROLLER_GAMEMANAGER}`)
const PORT_WSS_CLIENT: number = parseInt(`${process.env.PORT_WSS_CONTROLLER_CLIENT}`)
const PORT_WSS_RASPBERRY: number = parseInt(`${process.env.PORT_WSS_CONTROLLER_RASPBERRY}`)
let CONTROLLER_ACCESS: string = ""

// Temporary queue until Auth0+database is set up
// ws to close connection on POST /removeuser
const allowedUsers: Array<{"user_id": string, "playernumber": number, "ws": any}> = []
const prisma = new PrismaClient();  

//gets current users who are allowed to have their key inputs read, aka users in a match currently
const printCurrentUsers = () => {
    let output = ""
    allowedUsers.forEach((element) => {output += element["user_id"] + " "})
    return output
}

// WEBSOCKET RASPBERRY
// Make sure to set up Raspberry server first

// const ws_raspberry = new WebSocket(`ws://${PI_ADDR}:${PORT_WSS_RASPBERRY}`)
const ws_raspberry = new WebSocket(`ws://${PI_ADDR}:${PORT_WSS_RASPBERRY}`)

ws_raspberry.onopen = (event) => {
    console.log(`WS_RASPBERRY CONNECTED ws://${PI_ADDR}:${PORT_WSS_RASPBERRY}`)
}

ws_raspberry.onerror = (error) => {
    console.log("WS_RASPBERRY error: " + error.message) 
    console.error(error)
}
ws_raspberry.onclose = (event) => {
    console.log("WS_RASPBERRY closed")
}

// SECTION: EXPRESS SERVER: GAME_MANAGER -> CONTROLLER
// THIS IS A PRIVATE PORT
const app = express()
app.use(express.json())
//sets up use of CORS for communication
app.use(cors({
    origin: '*',
    methods: ['GET', 'POST', 'OPTIONS'],
    allowedHeaders: '*'
}));

//sets up what's allowed access. Allow localhost to communicate
app.options('*', (req, res) => {
    res.header('Access-Control-Allow-Origin', 'http://localhost:3000');
    res.header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.header('Access-Control-Allow-Headers', 'Content-Type, role');
    res.sendStatus(200);
});

//sets up controller to be listening
app.listen(PORT_SERVER, () => {
    console.log(`Express Server is running on http://${LOCALHOST}:${PORT_SERVER}`);
})

//when game manager sends the new access password to the controller, change it
app.post("/accesspassword", (request, response) => {
    CONTROLLER_ACCESS = request.body.accesspassword
    response.send(`CHANGED ACCESS PASSWORD TO ${CONTROLLER_ACCESS}`)
    console.log(`CHANGED ACCESS PASSWORD TO ${CONTROLLER_ACCESS}`)
})

//when game manager adds new user to the match, make controller aware
app.post("/adduser", (request, response) => {
    const user_id: string = request.body.user_id
    const playernumber: number = request.body.playernumber
    let status = 400
    // If allowedUsers.lenth < 2 and user is not in allowedUsers
    if(allowedUsers.length < 2 && allowedUsers.findIndex((element) => {return element["user_id"] === user_id}) == -1){
        // ws is set after the player connects through WebSocket. Makes user allowed user as well.
        allowedUsers.push({"user_id": user_id, "playernumber": playernumber, "ws": null})
        response.send(`ADDED PLAYER ${playernumber} as ${user_id}`)
        console.log(`ADDED PLAYER ${playernumber} as ${user_id} | ALLOWED_USERS: ${printCurrentUsers()}`)
        status = 200
    }
    response.status(status).end()
})

//when game manager says a user is removed from the match
app.post("/removeuser", (request, response) => {
    const user_id: string = request.body.user_id
    //finds index in the "allowedUsers" array of the user.
    const index = allowedUsers.findIndex((element) => {return element["user_id"] === user_id})
    let status = 400
    // If user is in allowedUsers
    if(index != -1){
        if(allowedUsers[index]["ws"]){ // Close ws connection if connected
            allowedUsers[index]["ws"].close()
        }
        //remove them from the array
        allowedUsers.splice(index, 1)
        response.send(`REMOVED ${user_id}`)
        console.log(`REMOVED ${user_id} | ALLOWED_USERS: ${printCurrentUsers()}`)
        status = 200
    }
    response.status(status).end()
})

//when multiple users are being added to the match
app.post("/addusers", (request, response) => {
    /* Example request body
        {
            "users":[{"user_id":"abc","playernumber":0},{"user_id":"def","playernumber":1}]
        }
    */
    const users = request.body
    console.log(users)
    // console.log(Object.keys(users)) // [ '0', '1' , ...]
    let status = 400
    //for each user being added, do thie following:
    Object.keys(users).forEach((key) => {
        const user_id = users[key]["user_id"]
        const playernumber = users[key]["playernumber"]
        //if allowed users is under 2, and they are in NOT already in the "allowedUsers" array, add them.
        if(allowedUsers.length < 2 && allowedUsers.findIndex((element) => {return element["user_id"] === user_id}) == -1){
            // ws is set after the player connects through WebSocket. Also add them to the array
            allowedUsers.push({"user_id": user_id, "playernumber": playernumber, "ws": null})
            // response.send(`ADDED PLAYER ${playernumber} as ${user_id}`)
            console.log(`ADDED PLAYER ${playernumber} as ${user_id} | ALLOWED_USERS: ${printCurrentUsers()}`)
            status = 200
        }
    })
    response.status(status).end()
})

//when removing multiple users at once from the match
app.post("/removeusers", (request, response) => {
    /* Example request body
    {
        "users":[{"user_id":"abc"},{"user_id":"def"}]
    }
    */
    const users = request.body.users
    // console.log(Object.keys(users)) // [ '0', '1' , ...]
    let status = 400
    //for each user being removed, do the following:
    Object.keys(users).forEach((key) => {
        const user_id = users[key]["user_id"]
        const index = allowedUsers.findIndex((element) => {return element["user_id"] === user_id})
        // If user is in allowedUsers
        if(index != -1){
            if(allowedUsers[index]["ws"]){ // Close ws connection if connected
                allowedUsers[index]["ws"].close()
            }
            //remove them from the array
            allowedUsers.splice(index, 1)
            // response.send(`REMOVED ${user_id}`)
            console.log(`REMOVED ${user_id} | ALLOWED_USERS: ${printCurrentUsers()}`)
            status = 200
        }
    })
    response.status(status).end()
})

//if the game manager tells the controller to shut down the robot:
app.post("/shutdownrobot", (request, response) => {
    const role = request.headers.role

    // Check if the user has an 'admin' role. If not, can't shut down as aunauthorized
    if (role !== "admin") {
        return response.status(403).json({ message: "Unauthorized" });
    }
    
    // If admin, send shutdown command to Raspberry Pi, by sending message to the ws_raspberry server
    ws_raspberry.send(JSON.stringify({
        "type": "ADMIN_INPUT",
        "payload": "ROBOT_SHUTDOWN"
    }))

    response.status(200).json({message: "Robot shutdown command sent!"});
});
 
//if game manager tells controller to edit match settings
app.post("/editMatchSettings", async (request, response) => {
    const role = request.headers.role;
    //gets new player count and match time
    const { numPlayers, matchTime } = request.body;

    // Ensure that the user is an admin. otherwise change fails
    if (role !== "admin") {
        return response.status(403).json({ message: "Unauthorized" });
    }

    try {
        // Upsert (update and insert) the matchSettings record

        //if a match settings object exists already, update match settings. otherwise create match settings object
        const updatedSettings = await prisma.matchSettings.upsert({
            where: { id: 1 },
            update: {
                numPlayers: parseInt(numPlayers),
                matchLength: matchTime,
            },
            create: {
                id: 1,
                numPlayers: parseInt(numPlayers),
                matchLength: matchTime,
            },
        });

        return response.status(200).json({
            message: "Match settings updated successfully.",
            data: updatedSettings,
        });
    } catch (error) {
        console.error(error);
        return response.status(500).json({ message: "Failed to update match settings."});
    }
});


// SECTION: WEBSOCKET SERVER: CLIENT -> CONTROLLER
const server = createServer()
const wss = new WebSocketServer({ noServer: true })

wss.on("error", (error) => {
    console.log("WebSocket server error: " + error)
})

wss.on("close", () => {
    console.log("WebSocket server closed")
})

//when a client connects
wss.on("connection", (ws: any, request: IncomingMessage, user_id: string) => {
    console.log("WSS_CONTROLLER_CLIENT: New connection!")
    //get index of user from the "allowedUsers" array
    const index = allowedUsers.findIndex((element) => { return element["user_id"] === user_id})
    const playernumber = allowedUsers[index]["playernumber"]

    //when a client sends a message, send it to the raspberry pi server (the one still on the website, not the pi itself)
    ws.on("message", (data: any) => {
    const msg = JSON.parse(data);
    const type = msg.type;
    const payload = msg.payload;

    if (type === "KEY_INPUT") {

        // --- VALIDATE ---
        if (typeof payload !== "string") return;
        if (!/^[01]{4}$/.test(payload)) return;

        console.log(`PLAYER ${playernumber}: ${user_id} | INPUT = ${payload}`);

        // --- FORWARD TO RASPBERRY ---
        ws_raspberry.send(JSON.stringify({
            type: "KEY_INPUT",
            payload: {
                keys: payload,
                playernumber
            }
        }));
    }
});

    allowedUsers[index]["ws"] = ws
    ws.send(JSON.stringify({ type: "CONNECTED" }))
})

// check for upgrade request to websocket from a logged in user
// different from a typical http request like GET or POST. But to do
// this, need to validate security stuff
server.on("upgrade", async (request, socket, head) => {
    try {
        // 1️⃣ Parse cookies
        const cookiesHeader = request.headers["cookie"] ?? "";
        if (!cookiesHeader) {
            socket.destroy();
            return;
        }

        const cookiePairs = cookiesHeader.split(";").map(c => c.split("="));
        const cookieObj: Record<string, string> = {};
        cookiePairs.forEach(([key, value]) => {
            if (key && value) {
                cookieObj[decodeURIComponent(key.trim())] = decodeURIComponent(value.trim());
            }
        });

        // 2️⃣ Check for session token and controller access
        const srtoken = cookieObj["srtoken"];
        const accesspassword = cookieObj["accesspassword"];
        if (!srtoken || !accesspassword) {
            socket.destroy();
            return;
        }

        if (accesspassword !== CONTROLLER_ACCESS) {
            socket.destroy();
            return;
        }

        // 3️⃣ Lookup user in database by session token
        const user = await prisma.player.findFirst({
            where: { sessionToken: srtoken }
        });

        if (!user) {
            socket.destroy();
            return;
        }

        // 4️⃣ Ensure user is allowed
        const allowedIndex = allowedUsers.findIndex(e => e.user_id === user.user_id);
        if (allowedIndex === -1) {
            socket.destroy();
            return;
        }

        // ✅ Upgrade to WebSocket
        wss.handleUpgrade(request, socket, head, (ws) => {
            wss.emit("connection", ws, request, user.user_id);
        });

    } catch (err) {
        console.error("WebSocket upgrade failed:", err);
        socket.destroy();
    }
});


server.listen(PORT_WSS_CLIENT, () => {
    console.log(`SERVER is running on http://${LOCALHOST}:${PORT_WSS_CLIENT}`)
})
