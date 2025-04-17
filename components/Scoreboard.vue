<!--creates the scoreboard for the current ongoing match.-->

<template>
  <div class="border-4 rounded-lg border-black flex justify-between items-center overflow-hidden scoreboard">
    <!--holds user one's score here.-->
    <p class="border-r-4 border-black rounded-lg bg-orange text-center score-box">{{ user1score }}</p>
    <!--holds user one's name.-->
    <p class="grow text-center player-name">{{ "SemiUltra" }}</p>
    <!--holds the timer for how much time is left in the match.-->
    <div class="bg-slate-300 border-black border-l-4 border-r-4 flex items-center timer-box">
      <p class="timer-text">{{ "1:37" }}</p>
    </div>
    <!--Holds user two's name here.-->
    <p class="grow text-center player-name">{{ "TheRealDhruv" }}</p>
    <!--holds user two's score here.-->
    <p class="border-l-4 border-black rounded-lg bg-green text-center score-box">{{ user2score }}</p>
  </div>
</template>

<script setup lang="ts">
//the parent component of this is the index.vue file, passes in both usernames, timer, and both scores.
import { computed } from "vue";

const props = defineProps({
    user1: {type: String, default: ""},
    user2: {type: String, default: ""},
    timer: {type: Number, default: 97},
    user1score: {type: Number, default: 0},
    user2score: {type: Number, default: 0}
})

//formats the timer to be displayed on the screen. Currently props.timer is just in seconds.
const formatTimer = computed(() => {
  //Divide timer by 60 to get time left.
  const minutes: number = Math.floor(props.timer / 60);
  //get remaining time after time per minute.
  const seconds = props.timer % 60;
  //convert the minutes and secnods to a string. Format is minutes:seconds, with padding of 3 digits total for minutes remaining and seconds remaining.
  return `${minutes.toString().padStart(2, "0")}:${seconds.toString().padStart(2, "0")}`
})
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