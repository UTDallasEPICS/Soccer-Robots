<!--The main file for the page, that has embedded in it all the UI components from the components folder.-->
<template>
  <div class="flex flex-col">
    <div>
      <TopNavBar></TopNavBar>
      <BottomNavBar></BottomNavBar>
    </div>
    <div class="flex justify-center py-6">
      <div class="flex flex-col">
        <!--sets up the scoreboard by passing in the timer variables, user1 and user2 variables, and the score variables.-->
        <Scoreboard :timer="Number(timer ?? 0)" :user1="player1?.username ?? ''" :user2="player2?.username ?? ''" :user1score="player1?.score ?? 0" :user2score="player2?.score ?? 0"></Scoreboard>
        <span class="py-4 pr-6">
                    <VideoStream></VideoStream>
                </span>
      </div>
      <!--when user tries to join or leave queue, run the according functions in this file.-->
      <QueueContainer :queueUsers="queue" @join-queue="joinQueue" @leave-queue="leaveQueue"></QueueContainer>
      <ConfirmMatchOverlay v-if="confirmationRequest && responseGiven" @confirm-response="confirmMatch"/>
    </div>
  </div>

</template>

<script setup lang="ts">
const showLogIn = ref(false)
//Srtoken is for security authentication, sruser is seeing if the user profile exists.
const srtoken = useCookie('srtoken')
const sruser = useCookie('sruser')

//if no user found from the cookie, show login section
if(srtoken.value != '' && sruser.value == ''){
  showLogIn.value = true
}

//Setting up two web sockets we have
const ws_queue = ref<WebSocket>()
const ws_controller = ref<WebSocket>()

//Confirmation request controls if we are going to show the overlay of asking the user to confirm they're ready. Initially it's false..
const confirmationRequest = ref(false)
const confirmationResponseGiven = ref(false)
//Sent to the client from the server, client then sends this back to show they've accepted. Used to tell clients apart from each other when confirming..
const confirmationPassword = ref("")
//Used to generate a private passsword for the client in their cookie when a match has started.
const accesspassword = ref("")

const sse = ref()
//setting our player queue
const queue = ref<Array<string>>()
const timer = ref<Number>(0)
//setting our timer
const player1 = ref<{score: number, username: string}>()
const player2 = ref<{score: number, username: string}>()

//Function to join queue
const joinQueue = () => {
  //first set up the websocket and the new link to it, using the localhost and gamemaster port.
  ws_queue.value = new WebSocket(`ws://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_CLIENT_GM}`)
  ws_queue.value.onerror = (event) => {
    console.log("Error: " + event)
    return
  }

  //when the websockets closed, send message saying as such
  ws_queue.value.onclose = (event) => {
    console.log("WebSocket connection closed")
  }


  //when the server sends the client a message
  ws_queue.value.onmessage = (event) => {
    const { type, payload } = JSON.parse(event.data)
    console.log(`Received message => ${type} : ${payload}`)
    //if server sent message for confirminig match, go into that mode. 
    if(type === "MATCH_CONFIRMATION") {
      confirmationPassword.value = payload
      confirmationRequest.value = true
    }
    //otherwise if resetting, put request as false.
    else if(type === "MATCH_CONFIRMATION_RESET"){
      confirmationRequest.value = false
    }
    //else if match starting, send password for the client in the match and make it a part of its cookie so they can return to the match.
    else if(type === "MATCH_START"){
      confirmationRequest.value = false
      accesspassword.value = payload
      document.cookie = "accesspassword=" + accesspassword.value
      //Creating a new websocket with the controller that is used to send inputs.
      ws_controller.value = new WebSocket(`ws://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_WSS_CONTROLLER_CLIENT}`)
      //when controller is open and created.
      ws_controller.value.onopen = (event) => {
        //initialize mapping of what inputs are pressed
        const wasdMapping: { [key: string]: number, "w": number, "a": number, "s": number, "d": number } = {"w": 0, "a": 0, "s": 0, "d": 0}
        
        //sets the key that was released back to 0 with the event. uses dictionary mapping, as event.key can return "w", "a", "s", or "d"
        const updateKeyUp = (event: KeyboardEvent) => {
          if(wasdMapping.hasOwnProperty(event.key)){
            wasdMapping[event.key] = 0
          }
        }

        //sets the key that was pressed to 1 with dictionary mapping.
        const updateKeyDown = (event: KeyboardEvent) => {
          if(wasdMapping.hasOwnProperty(event.key)){
            wasdMapping[event.key] = 1
          }
        }

        //listen on even for when a key is pressed or release, and if so call the function and pass in the event with the appropriate key.
        window.addEventListener("keyup", updateKeyUp)
        window.addEventListener("keydown", updateKeyDown)
        //runs the following function every 100 milliseconds
        const keyInputs = setInterval(() => {
          //if you have a connection with the game controller:
          if(ws_controller.value?.OPEN)
          {
            //the message being sent is 4 digit payload, corresponding to w, a, s, and d.
            const message = {
              type: "KEY_INPUT",
              payload: "" + wasdMapping["w"] + wasdMapping["a"] + wasdMapping["s"] + wasdMapping["d"]
            }
            //send pressed keys to backend
            ws_controller.value.send(JSON.stringify(message))
          }
          //if no connection with the game controller
          else{
            //stops the keyInputs function from looping
            clearInterval(keyInputs)
            //remove events as if not sending them, not needed
            window.removeEventListener("keyup", updateKeyUp)
            window.removeEventListener("keydown", updateKeyDown)
          }
        }, 100)
      }
    }
  }
  //when the socket to the queue server is first opened, send a message that the client was to join the queue
  ws_queue.value.onopen = (event) => {
    console.log("WebSocket connection established")
    //no need for payload here
    let message = {
      "type": "JOIN_QUEUE",
      "payload": ""
    }
    ws_queue.value?.send(JSON.stringify(message))
  }
}

//when leaving the queue
const leaveQueue = () => {
  //make sure there's a connection to the queue server. if so send message saying they wanna leave the queue
  if(ws_queue.value?.OPEN) {
    let message = {
      type: "LEAVE_QUEUE",
      payload: ""
    }
    ws_queue.value.send(JSON.stringify(message))
  }
}

//when the client has either accepted or denied the confirmation request, send that to the queue server
const confirmMatch = (accepted: boolean) => {
  if(ws_queue.value?.OPEN){
    ws_queue.value.send(JSON.stringify({
      type: "CONFIRMATION",
      payload: {"password": confirmationPassword.value, "accepted": accepted}
    }))
  }
}

//This one is for when the game manager sends data.
if(process.client){
  //creates the connection with the game manager. Don't have to be in the queue for this; just updates the queue, score, and timer
  sse.value = new EventSource(`http://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_SSE_GM}/sse-info`)
  //listens for when the game manager sends messages
  sse.value.addEventListener("message", (message: any) => {
    const data = JSON.parse(message.data)
    const type = data["type"]
    //if the message is to update queue, the payload has the new queue, so we update it
    if(type === "UPDATE_QUEUE"){
      const payload: Array<string> = data["payload"]
      queue.value = payload
    }
    //if the message is to update the timer on the scoreboard, we update that.
    else if(type === "UPDATE_TIMER"){
      const payload: number = data["payload"]
      timer.value = payload
    }
    //if the message is to update the score of the players, we do that.
    else if(type === "UPDATE_SCORE"){
      const payload: any = data["payload"]
      const { player1: p1, player2: p2 } : {player1: {score: number, username: string}, player2: {score: number, username: string}} = payload
      player1.value = p1
      player2.value = p2
    }
  })
}
</script>