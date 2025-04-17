

<template>
  <div class="flex md:flex md:flex-grow flex-row-reverse space-x-3 mt-5 mr-4 mb-2">
    <div class="">
      <button
          class="hover:text-gray-300 transition font-semibold duration-400 text-white font-bold py-2 px-5 rounded-lg"
          style="background-color:#154734" @click="login">Log In</button>
    </div>
  </div>
</template>

<script setup lang="ts">
const login = async () => { window.location.href = 'api/login' }
</script>

<style></style>