<template>
<div class="fixed w-full h-full inset-0 bg-opacity-20 backdrop-blur-sm flex" style="z-index:2;">
      <div class="rounded-lg p-4 bg-white border-2 border-black" style="width: 35vw; height: 90vh; margin: auto;">
        <div class="w-min h-min text-black" style="margin-left: auto;">
          <p @click="emitClose" class="cursor-pointer">X</p>
        </div>
        <h2 class="w-full text-center" style="font-size: 28px; font-weight: bold; margin-top:1em; margin-bottom: 1em;">About Us</h2>
        <p class="w-full text-left"> 
          Soccer Robots is an interactive, internet-controlled soccer game where two physical 
          robots compete in real-time while players take control remotely. Developed by students 
          at The University of Texas at Dallas through the Jonsson School UTDesign program, 
          the project brings together diverse talents from UTDesign Capstone, EPICS, Makerspace, 
          and Startup to create a fun, immersive robotics experience  
        </p>
        <p class="w-full text-left"> 
          Our goal is to build a robotics platform that’s not only exciting to play but 
          also serves as a hands-on learning tool. By combining real-time robot control
          with live streaming, we’re creating a unique blend of engineering, strategy, 
          and competitive fun—making robotics more accessible and engaging for everyone involved.

        </p>
        <p class="w-full text-left">
          Soccer Robots is more than a game — 
          it’s a gateway to STEM education, robotics 
          innovation, and hands-on engineering experience 
        </p>
      </div>
</div>
</template>
  
  <script setup lang="ts">
  const emit = defineEmits(['closeAboutUsOverlay'])
  const emitClose = () => {
      emit('closeAboutUsOverlay')
  }
  </script>
  
<style scoped>
.w-full p {
  font-size: 18px;
  color: #333;
  text-align: left;
  line-height: 1.2;
  margin-bottom: 2em
}
.w-full h2{
  font-size: 28px;
  font-weight: bold;
  line-height: 1.2;
  color: #1A3D2F;
  margin-bottom: 0.5rem;
}

</style>