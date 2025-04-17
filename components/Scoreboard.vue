<template>
  <div class="border-4 rounded-lg border-black flex justify-between items-center overflow-hidden scoreboard">
    <p class="border-r-4 border-black rounded-lg bg-orange text-center score-box">{{ user1score }}</p>
    <p class="grow text-center player-name">{{ "SemiUltra" }}</p>
    <div class="bg-slate-300 border-black border-l-4 border-r-4 flex items-center timer-box">
      <p class="timer-text">{{ "1:37" }}</p>
    </div>
    <p class="grow text-center player-name">{{ "TheRealDhruv" }}</p>
    <p class="border-l-4 border-black rounded-lg bg-green text-center score-box">{{ user2score }}</p>
  </div>
</template>

<script setup lang="ts">
import { computed } from "vue";

const props = defineProps({
  user1: { type: String, default: "" },
  user2: { type: String, default: "" },
  timer: { type: Number, default: 97 },
  user1score: { type: Number, default: 0 },
  user2score: { type: Number, default: 0 },
});

const formatTimer = computed(() => {
  const minutes: number = Math.floor(props.timer / 60);
  const seconds = props.timer % 60;
  return `${minutes.toString().padStart(2, "0")}:${seconds.toString().padStart(2, "0")}`;
});
</script>

<style scoped>

@import url('https://fonts.googleapis.com/css2?family=Jockey+One&family=Source+Code+Pro:ital,wght@0,700;1,700&display=swap');

.scoreboard {
  width: 65vw;
  height: 9vh;
  margin: 0 auto;
  margin-right: 2rem ;
}

.score-box {
  font-family: 'Jockey One', sans-serif;
  font-weight: 400;
  font-size: 5vw;
  min-width: 10vw;
  padding: 0.5vw;
}

.bg-orange {
  background-color: #E87500;
}

.bg-green {
  background-color: #5FE0B7;
}

.player-name {
  font-family: Helvetica, sans-serif;
  font-weight: bold;
  font-size: 2.5vw;
}

.timer-box {
  border-radius: 10px;
  border-width: 5px;
  padding: 0.5vw;
  width: 5.5vw;
  height: 5vh;
  display: flex;
  justify-content: center;
  align-items: center;
  color: C0C0C0;
}

.timer-text {
  font-family: 'Jockey One', sans-serif;
  font-weight: bold;
  font-size: 2vw;
}

</style>