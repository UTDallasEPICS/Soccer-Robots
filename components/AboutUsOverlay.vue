<template>
<div class="fixed w-full h-full inset-0 bg-opacity-20 backdrop-blur-sm flex" style="z-index:2;">
      <div class="rounded-lg p-2 bg-white" style="width: 35vw; height: 70vh; margin: auto;">
        <div class="w-min h-min text-black" style="margin-left: auto;">
          <p @click="emitClose" class="cursor-pointer">X</p>
        </div>
        <h2 class="w-full text-center" style="font-size: 28px; font-weight: bold; margin-top:1em; margin-bottom: 1em;">About Us</h2>
        <p class="w-full text-left" style="margin-bottom: 2em;"> 
          1.Lorem ipsum dolor sit amet, consectetur adipiscing elit. 
          Integer feugiat posuere quam. Maecenas vehicula ligula at gravida 
          faucibus. Pellentesque habitant morbi tristique senectus et netus 
        </p>
        <p class="w-full text-left"> 
          2.et malesuada fames ac turpis egestas. Pellentesque interdum 
          condimentum est nec fringilla. Nullam semper felis non interdum 
          faucibus. Etiam felis quam, ornare eget odio ultricies, pulvinar 
          pulvinar ligula. In semper congue ullamcorper. 
        </p>
        <img src="public/Robot.svg" alt="Soccer Robot Image (Credit: Zaaim Rahman)" style="display: block; margin: 0 auto; margin-top: .85em; margin-bottom: 0.85em;">
        <p class="w-full text-left">
          3.Lorem ipsum dolor sit amet, consectetur adipiscing elit.
          Integer feugiat posuere quam. Maecenas vehicula ligula at gravida faucibus. 
          Pellentesque habitant morbi tristique senectus et netus 
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
}
.w-full h2{
  font-size: 28px;
    font-weight: bold;
    color: #1A3D2F;
    margin-bottom: 0.5rem;
}

</style>