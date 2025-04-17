<template>
    <transition name="fade-blur">
    <div class="fixed w-full h-full inset-0 bg-opacity-20 backdrop-blur-sm flex" style="z-index: 2">
        <div class="rounded-lg p-2 bg-white" style="width: 500px; height: 400px; margin: auto;">
        <div class="w-min h-min text-black" style="margin-left: auto;">
            <p @click="emitClose" class="cursor-pointer" style="font-size: 1rem;">X</p>
        </div>
        <p class="w-full text-center" style="font-size: 1.75em; margin-bottom: 0.5em;">Help</p>
        <p class="w-full text-center">
            For assistance, please email
            <a href="mailto:Tim.Givens@utdallas.edu" style="text-decoration: underline; color: blue;">Tim.Givens@utdallas.edu</a>
        </p>
        <img src="public/TimGivens.jpg" alt="Tim Givens pictured here" style="display: block; margin: 0 auto; width: 50%; height: auto; margin-top: 1em;">
        </div>
    </div>
    </transition>
</template>

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