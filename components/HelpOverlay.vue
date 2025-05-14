<template>
    <transition name="fade-blur">
    <div class="fixed w-full h-full inset-0 bg-opacity-20 backdrop-blur-sm flex" style="z-index: 2">
        <div class="rounded-lg p-4 bg-white border-2 border-black" style="width: 500px; height: 400px; margin: auto;">
        
        <div class="w-min h-min text-black ml-auto">
            <p @click="emitClose" class="cursor-pointer text-base font-bold">X</p>
        </div>

        <p class="text-center text-[1.75em] mb-2 font-bold text-black">Help</p>

        <p class="text-center text-black">
            For assistance, please email
            <a href="mailto:Tim.Givens@utdallas.edu" class="underline text-blue-600">Tim.Givens@utdallas.edu</a>
        </p>

        <img src="/TimGivens.jpg" alt="Tim Givens pictured here" class="block mx-auto mt-4 w-1/2 h-auto" />
        </div>
    </div>
    </transition>
</template>

<script setup lang="ts">
const emit = defineEmits(['closeHelpOverlay'])
const emitClose = () => {
    emit('closeHelpOverlay')
}
</script>

<style scoped>
.fade-blur-enter-active,
.fade-blur-leave-active {
    transition: opacity 0.4s ease, backdrop-filter 0.4s ease;
}

.fade-blur-enter-from,
.fade-blur-leave-to {
    opacity: 0;
    backdrop-filter: blur(0px);
}
.fade-blur-enter-to,
.fade-blur-leave-from {
    opacity: 1;
    backdrop-filter: blur(4px);
}
</style>