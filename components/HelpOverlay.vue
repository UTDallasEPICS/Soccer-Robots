<!--overlay of the help mini-page-->
<template>
<div class="fixed w-full h-full inset-0 bg-black bg-opacity-80 flex" style="z-index: 2">
        <div class="rounded-lg p-2 bg-white" style="width: 500px; height: 400px; margin: auto;">
            <div class="w-min h-min text-black" style="margin-left: auto;">
                <p @click="emitClose" class="cursor-pointer">X</p>
            </div>
            <p class="w-full text-center" style="font-size: 1.75em; margin-bottom: 0.5em;">Help</p>
            <p class="w-full text-center">For assistance, please email 
                <a href="mailto:Tim.Givens@utdallas.edu" style="text-decoration: underline; color: blue;">Tim.Givens@utdallas.edu</a>
            </p>
            <img src="public/TimGivens.jpg" alt="Tim Givens pictured here" style="display: block; margin: 0 auto; width: 50%; height: auto; margin-top: 1em;">
        </div>
</div>
</template>

<script setup lang="ts">
const emit = defineEmits(['closeHelpOverlay'])
const emitClose = () => {
    emit('closeHelpOverlay')
}
</script>


