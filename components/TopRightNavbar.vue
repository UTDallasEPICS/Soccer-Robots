<template>
  <div class="">
    <!--Show the profile component if they're loggedi n, otherwise the component asking them to log in or sign up.-->
    <Profile v-if="userTrue"></Profile>
    <LogInSignUp v-else></LogInSignUp>
  </div>
  <!--Show this component if they need to choose a username.-->
  <LogInOverlay v-if="showLogIn" @closeLogIn="closing"></LogInOverlay>
</template>

<script setup lang="ts">
const userTrue = ref(false)
//Gets authentication cookie.
const srtoken = useCookie('srtoken')
//If able to get the cookie, means we have a user logged in already
if(srtoken.value){
  userTrue.value = true
}
const showLogIn = ref(false)
//gets cookie for the user itself
const sruser = useCookie('sruser')
//if the authentication worked but could not get the user, means they have to login
if(srtoken.value != '' && sruser.value == ''){
  showLogIn.value = true
}
//closes the login page
const closing = () => {
  showLogIn.value = false
  userTrue.value = true
}

</script>


<style>

</style>