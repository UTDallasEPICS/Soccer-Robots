<!-- need to actually make it look nice, it is literally two buttons-->
<template>
  <nav class="top-nav">
    <div class="logo-container">
      <img src="public/UTDLogo.svg" alt="UTD Logo" class="logo" />
    </div>
    <span class="title">Soccer Robots</span>
  </nav>

  <div class="landing">
    <h1>Welcome to Soccer Robots</h1>
    <p>Your ultimate soccer robot experience!</p>
    <p style="padding-top:100px">Choose your role to get started:</p>
    <p>Are you a Guest or a Player?</p>
    <p></p>
    <div class="buttons">
      <NuxtLink to="/guest">
        <button>I'm a Guest</button>
      </NuxtLink>

      <div v-if="isLoggedIn">
        <NuxtLink to="/player">
          <button>I'm a Player</button>
        </NuxtLink>
      </div>

      <div v-if="!isLoggedIn">
        <button @click="goToLogin">I'm a Player</button>
      </div>
    </div>
  </div>


</template>

<script setup>

const sruser = useCookie('sruser');
const isLoggedIn = computed(() => !!sruser.value)

const goToLogin = () => {
  window.location.href = '/api/login'; 
};

</script>

<style scoped>
.landing {
  padding-top: 50px;
  display: flex;
  flex-direction: column;
  align-items: center;
  height: 100vh;
  color: white;
  background-color: #154734;
  font-size: 3rem;
}
.buttons {
  display: flex;
  gap: 20px;
  margin-top: 20px;
}
button {
  font-size: 18px;
  padding: 10px 20px;
  margin-top: 100px;
  margin-left: 250px;
  margin-right: 250px;
  cursor: pointer;
  background-color: #f96c00;
}

.top-nav {
  width: 100%;
  padding: 16px 32px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  background-color: #f96c00;
  border-bottom: 2px solid #f96c00;
}

.logo-container {
  display: flex;
  align-items: center;
}

.logo {
  height: 70px;
  max-height: 90px;
  width: auto;
  object-fit: contain;
}
.title {
  font-weight: bold;
  font-size: 5rem;
  color: white;
  position: absolute;
  top: 0%;
  left: 50%;
  transform: translate(-50%, -10%);
}
</style>
