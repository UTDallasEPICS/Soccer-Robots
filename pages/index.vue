<template>
  <div class="oveoverflow-hidden h-screen ">
    <div>
      <BottomNavBar>  </BottomNavBar>
    </div>
    <div class="flex justify-center py-4">
      <div class="flex flex-col" style="margin-top: 3vh;">
        <Scoreboard :queue="queue" :user1score="player1?.score ?? 0" :user2score="player2?.score ?? 0" :timer="Number(timer ?? 0)"/>        <span class="py-4 pr-6">
                    <VideoStream></VideoStream>
                </span>
      </div>
      <QueueContainer :queueUsers="queue" @join-queue="joinQueue" @leave-queue="leaveQueue"></QueueContainer>
      <ConfirmMatchOverlay v-if="confirmationRequest" @confirm-response="confirmMatch"/>
    </div>
  </div>

</template>

<script setup lang="ts">
const showLogIn = ref(false)
const srtoken = useCookie('srtoken')
const sruser = useCookie('sruser')

if(srtoken.value != '' && sruser.value == ''){
  showLogIn.value = true
}

const closing = () => {
  showLogIn.value = false
}

const ws_queue = ref<WebSocket>()
const ws_controller = ref<WebSocket>()
const confirmationRequest = ref(false)
const confirmationPassword = ref("")
const accesspassword = ref("")

const sse = ref()
const queue = ref<Array<string>>()
const timer = ref<Number>(0)
const player1 = ref<{score: number, username: string}>()
const player2 = ref<{score: number, username: string}>()

const joinQueue = () => {
  ws_queue.value = new WebSocket(`ws://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_CLIENT_GM}`)
  ws_queue.value.onerror = (event) => {
    console.log("Error: " + event)
    return
  }
  ws_queue.value.onclose = (event) => {
    console.log("WebSocket connection closed")
  }
  ws_queue.value.onmessage = (event) => {
    const { type, payload } = JSON.parse(event.data)
    console.log(`Received message => ${type} : ${payload}`)
    if(type === "MATCH_CONFIRMATION") {
      confirmationPassword.value = payload
      confirmationRequest.value = true
    }
    else if(type === "MATCH_CONFIRMATION_RESET"){
      confirmationRequest.value = false
    }
    else if(type === "MATCH_START"){
      confirmationRequest.value = false
      accesspassword.value = payload
      document.cookie = "accesspassword=" + accesspassword.value
      ws_controller.value = new WebSocket(`ws://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_WSS_CONTROLLER_CLIENT}`)
      ws_controller.value.onopen = (event) => {
        const wasdMapping: { [key: string]: number, "w": number, "a": number, "s": number, "d": number } = {"w": 0, "a": 0, "s": 0, "d": 0}
        const updateKeyUp = (event: KeyboardEvent) => {
          if(wasdMapping.hasOwnProperty(event.key)){
            wasdMapping[event.key] = 0
          }
        }
        const updateKeyDown = (event: KeyboardEvent) => {
          if(wasdMapping.hasOwnProperty(event.key)){
            wasdMapping[event.key] = 1
          }
        }
        window.addEventListener("keyup", updateKeyUp)
        window.addEventListener("keydown", updateKeyDown)
        const keyInputs = setInterval(() => {
          if(ws_controller.value?.OPEN){

            const message = {
              type: "KEY_INPUT",
              payload: "" + wasdMapping["w"] + wasdMapping["a"] + wasdMapping["s"] + wasdMapping["d"]
            }
            ws_controller.value.send(JSON.stringify(message))
          }
          else{
            clearInterval(keyInputs)
            window.removeEventListener("keyup", updateKeyUp)
            window.removeEventListener("keydown", updateKeyDown)
          }
        }, 100)
      }
    }
  }
  ws_queue.value.onopen = (event) => {
    console.log("WebSocket connection established")
    let message = {
      "type": "JOIN_QUEUE",
      "payload": ""
    }
    ws_queue.value?.send(JSON.stringify(message))
  }
}

const leaveQueue = () => {
  if(ws_queue.value?.OPEN) {
    let message = {
      type: "LEAVE_QUEUE",
      payload: ""
    }
    ws_queue.value.send(JSON.stringify(message))
  }
}

const confirmMatch = (accepted: boolean) => {
  if(ws_queue.value?.OPEN){
    ws_queue.value.send(JSON.stringify({
      type: "CONFIRMATION",
      payload: {"password": confirmationPassword.value, "accepted": accepted}
    }))
  }
}

if(process.client){
  sse.value = new EventSource(`http://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_SSE_GM}/sse-info`)
  sse.value.addEventListener("message", (message: any) => {
    const data = JSON.parse(message.data)
    const type = data["type"]
    if(type === "UPDATE_QUEUE"){
      const payload: Array<string> = data["payload"]
      queue.value = payload
    }
    else if(type === "UPDATE_TIMER"){
      const payload: number = data["payload"]
      timer.value = payload
    }
    else if(type === "UPDATE_SCORE"){
      const payload: any = data["payload"]
      const { player1: p1, player2: p2 } : {player1: {score: number, username: string}, player2: {score: number, username: string}} = payload
      player1.value = p1
      player2.value = p2
    }
  })
}
</script>