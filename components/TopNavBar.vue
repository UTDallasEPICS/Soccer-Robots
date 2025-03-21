<!--Holds the overall top navigation bar here.-->
<template>
  <div class="flex flex-nowrap md:flex-grow shrink-0 space-x-3">
    <div class="utdLogo justify-start" style="width:450px">
      <!--UTD logo in the navigation bar.-->
      <img src="../public/UTDLogo.svg">
    </div>
    <div class="flex md:flex md:flex-grow flex-row-reverse space-x-3">
      <TopRightNavbar></TopRightNavbar>
    </div>
  </div>
</template>

<script setup lang="ts">
const showSignUp = ref(false)
const showLogIn = ref(false)

</script>
<style></style>