<template>
  <img :src="streamUrl" :width="width" :height="height" alt="Live stream" />
</template>

<script setup lang="ts">
const runtime = useRuntimeConfig()
console.log("Runtime config:", runtime)

const streamUrl = 'http://192.168.250.90:8000/stream.mjpg'
console.log("Stream URL:", streamUrl)

const width = 955
const height = 480
</script>

<style scoped>
img {
  border: 2px solid #000;
}
</style>