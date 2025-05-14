<template>
    <div class="flex md:flex md:flex-grow flex-row-reverse space-x-2 space-x-reverse mt-5 mr-2 mb-2">
        <div>
            <div class="">
            <button
                class="hover:text-gray-300 transition font-semibold duration-400 text-white font-bold py-2 px-2 rounded-lg"
                style="background-color:#FF0000" @click="logout"> {{ "Log Out" }}</button>
            </div>
        </div>
        <div class="profileTemp">
            <img src="../public/Profile.svg">
        </div>
        <div class="mt-1">
            <p>{{ username }}</p>
        </div>
    </div>

</template>

<script setup lang="ts">
    import type { Player } from '@prisma/client'
    const getUser = useCookie<Partial<Player>>('sruser')
    let username = getUser.value?.username as string

    const logout = async () => { 
        window.location.href='api/logout'
    }
</script>

<style setup lang="ts">
</style>