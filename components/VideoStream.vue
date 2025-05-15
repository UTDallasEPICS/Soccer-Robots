<template>
  <!--Defines video to be embedded into the website-->
  <div class="video-container">
    <iframe 
      :src="streamUrl" 
      frameborder="0" 
      allowfullscreen="false" 
      scrolling="no"  
      allow="autoplay; fullscreen"
      alt="Live stream"
    ></iframe>
  </div>
</template>

<script setup lang="ts">
const runtime = useRuntimeConfig()
console.log("Runtime config:", runtime)

const streamUrl = `http://${process.env.PI_ADDR}:${process.env.PI_CAMERA_PORT}/stream.mjpg`
console.log("Stream URL:", streamUrl)
</script>

<style scoped>
.video-container {
  max-width: 1250px;
  margin: auto;
  border: 5px solid black;
  border-radius: 20px;
  overflow: hidden;
  position: relative;
  padding-top: 56.25%; /* 16:9 Aspect Ratio */
}

.video-container iframe {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
}
</style>