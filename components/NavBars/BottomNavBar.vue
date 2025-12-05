
<template>
<div class="navbar dark:bg-[#154734]">
    <div class="logo-container">
        <img :src="logoSrc" alt="UTD Logo" class="utd-logo" />
    </div>
    <div class="Soccer-robots-container dark:text-[#C0C0C0]">
        <h2>Soccer Robots</h2>
    </div>  
    <div class="nav-links">
        <div class="nav-item dark:text-[#C0C0C0]" @click="openAboutUs">About</div>
            <div class="nav-item dark:text-[#C0C0C0]" @click="openHowToPlay">How to Play</div>
            <div class="nav-item dark:text-[#C0C0C0]" @click="openHelp">Help</div>
        <div>    
            <p class="nav-item dark:text-[#C0C0C0]" v-if="isLoggedIn" @click="openChangeUsername"> Change Username </p>
        </div>
        <div class="nav-item" v-if="isAdmin" @click="openAdminPanel">Admin</div>        
        <div> <Profile /> </div>
    </div>
    </div>
    <!-- overlays -->
    <AboutUsOverlay v-if="showAboutUs" @closeAboutUsOverlay="closeAboutUs" />
    <HowToPlayOverlay v-if="showHowToPlay" @closeHowToPlayOverlay="closeHowToPlay" />
    <HelpOverlay v-if="showHelp" @closeHelpOverlay="closeHelp" />
    <LeaderBoardOverlay v-if="showLeaderboard" @closeLeaderBoardOverlay="closeLeaderboard" />
    <LogInOverlay :isChangingUsername="isChangingUsername" v-if="showChangeUsername" @closeLogIn="closeChangeUsername" />
    <AdminPanel v-if="showAdminPanel && isAdmin" @closeAdminPanel="closeAdminPanel" />
</template>

<script setup lang="ts">

import { ref, computed, onMounted, onUnmounted } from 'vue'

const isDark = ref(false) // Start with a default

const updateDarkState = () => {
  isDark.value = document.documentElement.classList.contains('dark')
}

onMounted(() => {
  updateDarkState() // First check
  document.addEventListener('click', updateDarkState) // Re-check when clicked
})

onUnmounted(() => {
  document.removeEventListener('click', updateDarkState)
})

const logoSrc = computed(() =>
  isDark.value ? '/UTDLogo-dark.png' : '/UTDLogo.svg'
)

const sruser = useCookie('sruser');

const isLoggedIn = computed(() => !!sruser.value)
const isChangingUsername = computed(() => true)
const isAdmin = computed(() => sruser.value?.role === 'admin')

const showAboutUs = ref(false)
const showHowToPlay = ref(false)
const showHelp = ref(false)
const showLeaderboard = ref(false)
const showChangeUsername = ref(false)
const showAdminPanel = ref(false)

const openAboutUs = () => { showAboutUs.value = true }
const closeAboutUs = () => { showAboutUs.value = false }
const openHowToPlay = () => { showHowToPlay.value = true }
const closeHowToPlay = () => { showHowToPlay.value = false }
const openHelp = () => { showHelp.value = true }
const closeHelp = () => { showHelp.value = false }
const openLeaderboard = () => { showLeaderboard.value = true }
const closeLeaderboard = () => { showLeaderboard.value = false }
const openChangeUsername = () => { showChangeUsername.value = true }
const closeChangeUsername = () => { showChangeUsername.value = false }
const openAdminPanel = () => { showAdminPanel.value = true }
const closeAdminPanel = () => { showAdminPanel.value = false }
</script>

<style>
.navbar {
    background-color: #f96c00; 
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0.5rem 0.5rem;
    height: 8vh;
    width: 100%;
}

.logo-container {
    display: flex;
    align-items: center;
}
.Soccer-robots-container{
    position: absolute;
    font-family:'Roboto Mono', sans-serif ;
    font-size: 2.5rem;
    font-weight: bold;
    color: white;
    margin-left: 10vh;    
}

.utd-logo {
    height: 50px;
    width: auto;
}

.nav-links {
    display: flex;
    gap: 1.5rem;
    margin-right: 1.5rem;
    border-color: black;
}

.nav-item {
    display: flex;
    font-family: 'Roboto Mono', sans-serif;
    font-size: 1.5rem;
    font-weight: bold;
    margin-right: 1vh;
    color: white;
    cursor: pointer;
    transition: color 0.1s ease-in-out;
    margin-top: 1.2rem;
}
.nav-login{
    font-family: 'Roboto Mono', sans-serif;
    font-size: 1rem;
    font-weight: bold;
    color: white;
    cursor: pointer;
    transition: color 0.1s ease-in-out;
}

.nav-item:hover {
    color: #d6d6d6;
}

/* Responsive Design */
@media (max-width: 768px) {
    .navbar {
        flex-direction: column;
        text-align: center;
    }

    .nav-links {
        flex-direction: column;
        gap: 1rem;
    }

    .utd-logo {
        height: 40px;
    }
}
</style>