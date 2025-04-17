<template>
  <!--Defines video to be embedded into the website-->
  <div class="rounded-lg overflow-hidden shadow-lg" style="border-radius: 20px; border-width: 5px; border-color: black;">
    <iframe :src="src" frameborder="0" allowfullscreen="false" scrolling="no"  :height="height" :width="width"></iframe>
  </div>

</template>

<script setup lang="ts">
const runtime = useRuntimeConfig()
//Sets the current steram shown on the screen to what is defined as the channel name. 
const src = `https://player.twitch.tv/?channel=${runtime.CHANNEL_NAME}&parent=${runtime.PARENT_NAME}` 
const height = 550
const width = 1000
</script>

<style>

</style>
