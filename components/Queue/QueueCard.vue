<!--Creates the queue card, which tells who is going to be against who in the upcoming matches.-->

<template>
    <div class="flex flex-no-wrap place-content-evenly" style = "padding: 1.5%;">
        <p class="basis-0 grow truncate text-center" style="font-weight: bold; font-size: 15px; font-family: Inter;"> {{ user1 }} </p>
        <p class="w-min" style="font-weight: bold; font-size: 15px; font-family: Inter"> VS </p>
        <p class="basis-0 grow truncate text-center" style="font-weight: bold; font-size: 15px; font-family: Inter;"> {{ user2 }} </p>
    </div>
    </template>
        
    <script setup lang="ts">
//define that into this component which is a child of Queue, what needs to be passed in. Here, we need the two user names.
    const props = defineProps({
        user1: String,
        user2: String
    })
    </script>
    
    <style>
    @import url('https://fonts.googleapis.com/css2?family=Inter:wght@100..900&display=swap')
    </style>