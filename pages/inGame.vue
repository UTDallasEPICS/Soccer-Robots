<template>
  <div class="guest-page">
    <!-- Top Navigation Bar -->
    <TopNavBar
      @toggle-help="showHelp = !showHelp"
      @toggle-how="showHowToPlay = !showHowToPlay"
    />

    <!-- Scoreboard -->
    <div class="scoreboard-container">
      <Scoreboard :timer="Number(timer ?? 0)" :user1="player1?.username ?? ''" :user2="player2?.username ?? ''" :user1score="player1?.score ?? 0 " :user2score="player2?.score ?? 0"></Scoreboard>
    </div>

    <!-- Twitch Stream -->
    <div class="stream-container">
      <VideoStream streamType="janus" />
    </div>

    <!-- Overlays -->
    <HelpOverlay v-if="showHelp" @closeHelpOverlay="showHelp = false" />
    <HowToPlayOverlay v-if="showHowToPlay" @closeHowToPlayOverlay="showHowToPlay = false" />
  </div>
</template>


<!-- Import the compnents needed -->
<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue';
import { useRouter } from 'vue-router'
import { useRuntimeConfig } from '#app';

import TopNavBar from '@/components/TopNavBar.vue';
import Scoreboard from '@/components/Scoreboard.vue';
import VideoStream from '@/components/VideoStream.vue';

import HelpOverlay from '@/components/HelpOverlay.vue';
import HowToPlayOverlay from '@/components/HowToPlayOverlay.vue';

const showHelp = ref(false);
const showHowToPlay = ref(false);

const queue = ref([]);

const accesspassword = useCookie("accesspassword")
const router = useRouter()
const ws_controller = ref<WebSocket>()

const sse = ref()

onMounted(() => {
  /*
  if (!accesspassword.value) {
    // If someone tries to go to /inGame directly
    router.push("/player")
    return
  }
  */

  ws_controller.value = new WebSocket(`ws://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_WSS_CONTROLLER_CLIENT}`)

  ws_controller.value.onopen = () => {
    const wasdMapping: { [key: string]: number, "w": number, "a": number, "s": number, "d": number } = {"w": 0, "a": 0, "s": 0, "d": 0}
    const updateKeyUp = (event: KeyboardEvent) => {
        
      if(wasdMapping.hasOwnProperty(event.key)){
        wasdMapping[event.key] = 0
        keyInputs()
      }
    }
        
    const updateKeyDown = (event: KeyboardEvent) => {
      if(event.repeat){
        console.log("repeating")
      }
      else if(wasdMapping.hasOwnProperty(event.key)){
        wasdMapping[event.key] = 1
        keyInputs()
      }
    }
    
    window.addEventListener("keyup", updateKeyUp)
    window.addEventListener("keydown", updateKeyDown)

    const keyInputs = () => {
      if(ws_controller.value?.OPEN){

        const message = {
          type: "KEY_INPUT",
          payload: "" + wasdMapping["w"] + wasdMapping["a"] + wasdMapping["s"] + wasdMapping["d"]
        }
        console.log("keyinput")

        ws_controller.value.send(JSON.stringify(message))
      }
    }

    onUnmounted(() => {
      window.removeEventListener("keydown", updateKeyDown)
      window.removeEventListener("keyup", updateKeyUp)
    })
    
  }
}
)

//This one is for when the game manager sends data.
const timer = ref<Number>(0)
const player1 = ref<{score: number, username: string}>()
const player2 = ref<{score: number, username: string}>()

if(process.client){
  //creates the connection with the game manager. Don't have to be in the queue for this; just updates the queue, score, and timer
  sse.value = new EventSource(`http://${useRuntimeConfig().public.LOCALHOST}:${useRuntimeConfig().public.PORT_SSE_GM}/sse-info`)
  //listens for when the game manager sends messages
  sse.value.addEventListener("message", (message: any) => {
    const data = JSON.parse(message.data)
    const type = data["type"]
    //if the message is to update the timer on the scoreboard, we update that.
    if(type === "UPDATE_TIMER"){
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

<style scoped>
.guest-page {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 0;
  gap: 16px;
}

.scoreboard-container {
  width: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  margin-top: 16px;
}

.scoreboard-container :deep(*) {
  margin-left: auto;
  margin-right: auto;
}

.stream-container {
  width: 100%;
  display: flex;
  justify-content: center;
  padding-bottom: 20px;
}

.queue-container {
  width: 100%;
  display: flex;
  justify-content: center;
  padding-bottom: 40px;
}

iframe, video {
  width: 100%;
  max-width: 960px;
  aspect-ratio: 16 / 9;
  border: none;
}
</style>