
<script setup lang="ts" >
import type {Player} from "@prisma/client"
const players = ref<Player[]>([])

const sortOption = ref('wins')
// Get data about the player from the leaderboard with fetch request.
const {data: playerData} = await useFetch('/api/leaderboard', {
   method: 'GET',
   query: {sortedColumn: sortOption},
   watch: [sortOption]
  })

const emit = defineEmits(['closeLeaderBoardOverlay'])
//when this function is called, close leaderboard
const emitClose = () => {
    emit('closeLeaderBoardOverlay')
}
</script>

<template>
  <div class="fixed w-full h-full inset-0 bg-black bg-opacity-80 flex" style="z-index: 2">
    <div class="rounded-lg p-2 bg-white" style="width: 500px; height: 400px; margin: auto;">
      <div class="w-min h-min text-black" style="margin-left: auto;">
          <!--if clicking x, close leaderboard--> 
          <p @click="emitClose" class="cursor-pointer">X</p>
      </div>
              
    <label for="sortOption">Sort by:</label>
    <select v-model="sortOption">
      <option value="goals">Goals</option>
      <option value="wins">Wins</option>
      <option value="losses">Losses</option>
      <option value="ratio">W/L Ratio</option>
    </select>
      <table class="w-full border-collapse mt-5">
      <thead>
        <tr class = "p-2.5" style="border: 1px solid #ccc">
          <th>#</th>
          <th>Name</th>
          <th>Goals</th>
          <th>Wins</th>
          <th>Losses</th>
          <th>W/L Ratio</th>
        </tr>
      </thead>
      <!--corresponding to each category defined in thead, show the data of the user in regards to each category.-->
      <tbody>
        <tr class="text-center" v-for="(player, index) in playerData" :key="index">
          <td>{{ index + 1 }}</td>
          <td>{{ player.username }}</td>
          <td>{{ player.goals }}</td>
          <td>{{ player.wins }}</td>
          <td>{{ player.losses }}</td>
          <td>{{ player.ratio }}</td>
        </tr>
      </tbody>
    </table>
            </div>
  </div>
  </template>