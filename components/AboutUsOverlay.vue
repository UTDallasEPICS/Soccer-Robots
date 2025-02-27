<!--This is the overlay for the about us page.-->
<template>
<div class="fixed w-full h-full inset-0 bg-black bg-opacity-80 flex" style="z-index: 2">
      <div class="rounded-lg p-2 bg-white" style="width: 500px; height: 400px; margin: auto;">
        <div class="w-min h-min text-black" style="margin-left: auto;">
          <p @click="emitClose" class="cursor-pointer">X</p>
        </div>
        <p class="w-full text-center" style="font-size: 28px; margin-bottom: 1em;">About Us</p>
        <p class="w-full text-center">Team Shawarama along with RL^2 presents to you: Soccer Robots! These fun, interactive robots allow you to play soccer from the comforts of your own phone. Using the WASD keys on your keyboard, you can move your robot to the World Cup!</p>
        <img src="public/RobotSketch.png" alt="Soccer Robot Image (Credit: Zaaim Rahman)" style="display: block; margin: 0 auto; width: 35%; height: auto; margin-top: .75em;">
        <p class="w-full text-center">Image credit: Zaaim Rahman (Engineering Team)</p>
      </div>
</div>
</template>
  
  <script setup lang="ts">
  const emit = defineEmits(['closeAboutUsOverlay'])
  const emitClose = () => {
      emit('closeAboutUsOverlay')
  }
  </script>
  