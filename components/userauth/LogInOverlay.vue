<!-- this mini-page is used when either setting your username for the first time OR changing your username-->
<template>
  <div class="fixed w-full h-full inset-0 bg-opacity-20 backdrop-blur-sm flex" style="z-index: 2;">
    <div class="rounded-lg p-4 bg-white border-2 border-black" style="margin: auto; width: 35%; height: 45%;">
      
      <div v-if="props.isChangingUsername" class="w-min h-min ml-auto text-black">
        <p @click="emitClose" class="cursor-pointer font-bold text-lg">X</p>
      </div>

      <p class="text-black font-black text-lg text-center" style="font-family: Inter; color: #154734; margin-top: 10%; margin-bottom: 11%;">
        Set Username
      </p>

      <form @submit.prevent="handleSubmit">
        <label class="font-semibold text-lg block ml-[14.5%]" style="font-family: Inter; color: #777070; letter-spacing: 1.5px;">
          USERNAME
        </label>

        <p class="text-red-600 ml-16 mt-2">{{ mssg }}</p>
        <!--Set what they say equal to the value "username" with v-model = username-->
        <input
          type="text"
          v-model="username"
          required
          class="text-black border-2 border-black p-5 font-semibold text-sm mt-5"
          placeholder="Enter username here"
          style="border-radius: 20px; border-color: #B6B6B6; width: 80%; margin-left: 10.5%; letter-spacing: 1.5px; font-family: Inter;"
        />

        <button
          type="submit"
          class="text-white border p-4 font-semibold text-lg tracking-widest"
          style="background-color: #E87500; border-radius: 20px; width: 65%; margin-left: 17%; margin-top: 5%; font-family: Inter;"
        >
          Set Username
        </button>
      </form>
    </div>
  </div>
</template>

<script setup lang="ts">
//LogInOverlay is the child of the BottomNavBar component, here the overlay will pass in a boolean for if the username si being changed.
import { ref } from 'vue'

const props = defineProps({
  isChangingUsername: { type: Boolean }
})

const username = ref("")
const mssg = ref("")

const emit = defineEmits(['closeLogIn'])

const emitClose = () => {
  emit('closeLogIn')
}

//funciton to send username to see if it's valid and if so, set it
const handleSubmit = async () => {
  console.log(props.isChangingUsername)
  //if not in range, return
  if(username.value.length < 3 || username.value.length > 15)
  {
    mssg.value = "Username has to be between 3 and 15 characters"
    return
  }

  //if username valid, try to set it.

  const req:string = await $fetch('api/user', {
    method:'put',
    body: {
      username: username.value
    }
  })
  //if no error, close this layer and refresh.
  if(parseInt(req) == 200)
  {
    emitClose()
    reloadNuxtApp()
  } 
  //if get an error, assume the error was because the username already exists.
  else
  {
    mssg.value = "Username already exists"
    return;
  }
}

//we define an emit that will close this layer and emitClose is the function we call to close it.
</script>

<style>
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@100..900&display=swap');
</style>