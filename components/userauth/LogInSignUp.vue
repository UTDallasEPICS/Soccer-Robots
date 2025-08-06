<template>
  <div class="flex md:flex md:flex-grow flex-row-reverse space-x-3 mt-5 mr-4 mb-2">
    <div>
      <button
        @click="login"
        class="transition font-semibold duration-400 text-white dark:text-[#C0C0C0] font-bold py-2 px-5 rounded-lg bg-[#154734] dark:bg-black hover:text-gray-300"
      >
        Become Player
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
const login = async () => {
  window.location.href = 'api/login';
};
</script>
