<!--creates the scoreboard for the current ongoing match.-->

<template>
  <div class="border-4 rounded-lg border-black flex justify-between items-center overflow-hidden" style="width: 959px; height: 69px; margin-top: 0%;">
    <!--holds user one's score here.-->
    <p class="border-r-4 rounded-r-none border-black rounded-lg bg-auto" style="font-family: Jockey One; font-weight: 400; font-size: 60px; background-color: #E87500 ;width: 130px; text-align: center;">{{ user1score }}</p>
    <!--holds user one's name.-->
    <p class="basis-0 grow text-center" style="font-family: Helvetica; font-weight: bold; font-size: 30px">{{user1}}</p>
    <!--holds the timer for how much time is left in the match.-->
    <div class="bg-slate-300 w-min border-black border-l-4 border-r-4 h-full px-3 h-full flex items-center">
      <p class="" style="font-family: Helvetica; font-weight: bold; font-size: 30px">{{ formatTimer }}</p>
    </div>
    <!--Holds user two's name here.-->
    <p class="basis-0 grow text-center" style="font-family: Helvetica; font-weight: bold; font-size: 30px">{{user2}}</p>
    <!--holds user two's score here.-->
    <p class="border-l-4 rounded-l-none border-black rounded-lg bg-auto" style="font-family: Jockey One; font-weight: 400; font-size: 60px; background-color: #5FE0B7; width: 130px; text-align: center;">{{ user2score }}</p>
  </div>
</template>

<script setup lang="ts">
//the parent component of this is the index.vue file, passes in both usernames, timer, and both scores.
const props = defineProps({
    user1: {type: String, default: ""},
    user2: {type: String, default: ""},
    timer: {type: Number, default: 0},
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

<style>
@import url('https://fonts.googleapis.com/css2?family=Jockey+One&display=swap')
</style>
