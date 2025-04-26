<template>
    <div class="navbar">
        <div class="logo-container">
            <img src="/public/UTDLogo.svg" alt="UTD Logo" class="utd-logo" />
        </div>
        <div class = "Soccer-robots-container" >
            <h2>Soccer Robots</h2>
        </div>  
        <div class="nav-links">
            <div class="nav-item" @click="openAboutUs">About</div>
            <div class="nav-item" @click="openHowToPlay">How to Play</div>
            <div class="nav-item" @click="openHelp">Help</div>
            <div class="nav-login"> <TopRightNavbar> </TopRightNavbar> </div>
        </div>
    </div>

    <AboutUsOverlay v-if="showAboutUs" @closeAboutUsOverlay="closeAboutUs" />
    <HowToPlayOverlay v-if="showHowToPlay" @closeHowToPlayOverlay="closeHowToPlay" />
    <HelpOverlay v-if="showHelp" @closeHelpOverlay="closeHelp" />
    <LeaderBoardOverlay v-if="showLeaderboard" @closeLeaderBoardOverlay="closeLeaderboard" />
    <LogInOverlay :isChangingUsername="isChangingUsername" v-if="showChangeUsername" @closeLogIn="closeChangeUsername" />
    <AdminPanel v-if="showAdminPanel" @closeAdminPanel="closeAdminPanel" />
</template>

<script setup lang="ts">
const sruser = useCookie('sruser');
const isLoggedIn = computed(() => sruser.value);
const isChangingUsername = computed(() => true);

const showAboutUs = ref(false);
const showHowToPlay = ref(false);
const showHelp = ref(false);
const showLeaderboard = ref(false);
const showChangeUsername = ref(false);
const showAdminPanel = ref(false);

const openAboutUs = () => { showAboutUs.value = true; };
const closeAboutUs = () => { showAboutUs.value = false; };
const openHowToPlay = () => { showHowToPlay.value = true; };
const closeHowToPlay = () => { showHowToPlay.value = false; };
const openHelp = () => { showHelp.value = true; };
const closeHelp = () => { showHelp.value = false; };
const openLeaderboard = () => { showLeaderboard.value = true; };
const closeLeaderboard = () => { showLeaderboard.value = false; };
const openChangeUsername = () => { showChangeUsername.value = true; };
const closeChangeUsername = () => { showChangeUsername.value = false; };
const openAdminPanel = () => { showAdminPanel.value = true; };
const closeAdminPanel = () => { showAdminPanel.value = false; };
</script>

<style scoped>
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
    display: flex;
    font-family:'Roboto Mono', sans-serif ;
    font-size: 2.5rem;
    font-weight: bold;
    color: white;
    margin-right: 70vh;    
}

.utd-logo {
    height: 50px;
    width: auto;
}

.nav-links {
    display: flex;
    gap: 3rem;
    margin-right: 1.5rem;
    border-color: black;
}

.nav-item {
    font-family: 'Roboto Mono', sans-serif;
    font-size: 1.5rem;
    font-weight: bold;
    color: white;
    cursor: pointer;
    transition: color 0.3s ease-in-out;
    margin-top: 1.2rem;
}
.nav-login{
    font-family: 'Roboto Mono', sans-serif;
    font-size: 1rem;
    font-weight: bold;
    color: white;
    cursor: pointer;
    transition: color 0.3s ease-in-out;
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
