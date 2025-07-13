<!-- need to actually make it look nice, it is literally two buttons-->
<template>
  <div class="landing">
    <h1>Welcome to Soccer Robots</h1>
    <p>Your ultimate soccer robot experience!</p>
    <p>Choose your role to get started:</p>
    <p>Are you a Guest or a Player?</p>
    <p></p>
    <div class="buttons">
      <NuxtLink to="/guest">
        <button>I'm a Guest</button>
      </NuxtLink>
      <NuxtLink to="/player">
        <button>I'm a Player</button>
      </NuxtLink>
    </div>
  </div>
</template>
<script setup>
</script>

<style scoped>
.landing {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100vh;
}
.buttons {
  display: flex;
  gap: 20px;
  margin-top: 20px;
}
button {
  font-size: 18px;
  padding: 10px 20px;
  cursor: pointer;
}
</style>
