<template>
  <div class="Login">
    <Profile v-if="userTrue"></Profile>
    <LogInSignUp v-else></LogInSignUp>
  </div>
  <LogInOverlay v-if="showLogIn" @closeLogIn="closing"></LogInOverlay>
</template>

<script setup lang="ts">
const userTrue = ref(false)
const srtoken = useCookie('srtoken')
if(srtoken.value){
  userTrue.value = true
}
const showLogIn = ref(false)
const sruser = useCookie('sruser')

if(srtoken.value != '' && sruser.value == ''){
  showLogIn.value = true
}

const closing = () => {
  showLogIn.value = false
  userTrue.value = true
}

</script>


<style scoped>
</style>