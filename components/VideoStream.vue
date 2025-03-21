<template>
  <!--Defines video to be embedded into the website-->
  <iframe :src="src" frameborder="0" allowfullscreen="false" scrolling="no" :height="height" :width="width"></iframe>
</template>

<script setup lang="ts">
const runtime = useRuntimeConfig()
//Sets the current steram shown on the screen to what is defined as the channel name. 
const src = `https://player.twitch.tv/?channel=${runtime.CHANNEL_NAME}&parent=${runtime.PARENT_NAME}` 
const height = 540
const width = 959
</script>

<style>

</style>
