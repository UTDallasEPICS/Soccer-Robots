<!-- made cleaner and more minimal look, might get adjusted-->
<template>
  <nav class="top-nav">
    <div class="logo-container">
      <img src="public/UTDLogo.svg" alt="UTD Logo" class="logo" />
      <span class="title">Soccer Robots</span>
    </div>

    <div class="nav-links">
      <div v-if="isGuest">
        <button @click="$emit('toggle-about')">About</button>
      </div>
      <button @click="$emit('toggle-how')">How to Play</button>
      <button @click="$emit('toggle-help')">Help</button>
      <div v-if="isGuest">
        <div v-if="!isLoggedIn">
          <button @click="goToLogin">Log In</button>
        </div>
        <div v-if="isLoggedIn">
          <NuxtLink to="/player">
            <button>Become Player</button>
          </NuxtLink>
        </div>
      </div>
    </div>
  </nav>
</template>

<script setup>
const goToLogin = async () => {

  const email = prompt("Please enter your email address:");
  if (!email) {
    alert("Email is required to log in.");
    return;
  }

  try {
    await $fetch('/api/login-request', {
      method: 'POST',
      body: { email }
    });
    alert("Login link sent! Please check your email.");
  } catch (error) {
    console.error("Error requesting login link:", error);
    alert("Failed to send login link. Please try again later.");
  }
  
};

const sruser = useCookie('sruser');
const isLoggedIn = computed(() => !!sruser.value)

import { useRoute } from 'vue-router'

const route = useRoute()
const isGuest = computed(() => route.path === '/' || route.name === 'guest')

</script>

<style scoped>
.top-nav {
  width: 100%;
  padding: 16px 32px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  background-color: white;
  border-bottom: 2px solid #ddd;
}

.logo-container {
  display: flex;
  align-items: center;
}

.logo {
  height: 40px;
  max-height: 60px;
  width: auto;
  object-fit: contain;
}
.title {
  font-weight: bold;
  font-size: 1.25rem;
  color: #ff6600;
}

.nav-links button {
  margin-left: 16px;
  background-color: #004d26;
  color: white;
  padding: 8px 16px;
  border-radius: 6px;
  font-weight: 500;
  border: none;
  cursor: pointer;
}

.nav-links button:hover {
  background-color: #006b36;
}
</style>
