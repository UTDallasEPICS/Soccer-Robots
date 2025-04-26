<!--used for the overlay when two people are queued up and want to confirm a match-->
<template>
<div class="fixed w-full h-full inset-0 bg-black bg-opacity-80 flex" style="z-index: 2;">
    <div class="w-min h-min rounded-lg p-2 bg-gray-500 bg-opacity-90" style="margin: auto;">
        <!--Passing in the params for the circle-->
        <ConfirmMatchCircle :radius="250" :progress="progress" :stroke="50"/>
        <!--on whichever is clicked, do the proper response.-->
        <button @click="$emit('confirm-response', true)" class = "border-4 rounded-2xl border-black content-center" style="font-size: 24px; font-weight: bold; width: 100%; background-color: #5FE0B7">
            ACCEPT!
        </button>
        <button @click="$emit('confirm-response', false)" class = "border-4 rounded-2xl border-black content-center" style="font-size: 24px; font-weight: bold; width: 100%; background-color: #FF0000">
            DECLINE!
        </button>
    </div>
</div>

</template>

<script setup lang="ts">
const emit = defineEmits(['confirm-response'])
const updateDelayms: number = 500



const confirmationTime: number = parseInt(useRuntimeConfig().public.CONFIRMATION_TIMER_DURATION) // in seconds
//how many ticks does the circular bar update by. So if 5 ticks, each tick the bar jumps +20% from its previous value. Basically
//the circular bar is not continuoulsy increasing.
const confirmationTicks: number = confirmationTime * (1000 / updateDelayms)
let confirmationTicksLeft: number = confirmationTicks

const progress = ref<number>(0)
//once this is activated
onMounted(() => {
    confirmationTicksLeft = confirmationTicks
    //every duration of updateDelay ms, we run what's in here to update it.
    const interval = setInterval(() => {
        confirmationTicksLeft--
        // This one decreases over time
        progress.value = (confirmationTicksLeft / confirmationTicks) * 100
        //once done, end this setInterval loop, and then leave onMoutned loop.
        if(confirmationTicksLeft === 0){
            clearInterval(interval);
        }
    }, updateDelayms);
})
</script>