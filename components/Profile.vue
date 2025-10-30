<!-- template to show the username, profile picture, and logout button -->
<template>
    <div class="flex md:flex md:flex-grow flex-row-reverse space-x-2 space-x-reverse mt-5 mr-2 mb-2">
        <div>
            <div class="">
            <!--We have  a button where on click, we'll call the logout function.-->
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
    //to get play info like username, we use the cookie we have of them and find their username from it
    const getUser = useCookie<Partial<Player>>('sruser')
    let username = getUser.value?.username as string
    //send the user to the logout page.
    const logout = async () => { 
        window.location.href='api/logout'
    }
</script>

<style setup lang="ts">
</style>