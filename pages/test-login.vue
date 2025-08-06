<template>
  <div class="test-login">
    <h2>Magic Link Tester</h2>
    <input v-model="email" placeholder="Enter email" />
    <button @click="sendMagicLink">Send Link</button>
    <p v-if="status === 'success'"> Link sent! Check the server log.</p>
    <p v-if="status === 'error'"> Error sending link.</p>
  </div>
</template>

<script setup>
import { ref } from 'vue'

const email = ref('')
const status = ref('')

async function sendMagicLink() {
  try {
    await $fetch('/api/login-request', {
      method: 'POST',
      body: { email: email.value }
    })
    status.value = 'success'
    console.log(' Link sent. Check terminal output.')
  } catch (err) {
    console.error(' Failed to send:', err)
    status.value = 'error'
  }
}
</script>

<style scoped>
.test-login {
  max-width: 500px;
  margin: 2rem auto;
  padding: 1rem;
  border: 1px solid #ccc;
  text-align: center;
}
input {
  padding: 0.5rem;
  width: 80%;
  margin-bottom: 1rem;
}
button {
  padding: 0.5rem 1rem;
}
</style>
