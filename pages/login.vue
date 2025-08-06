<template>
  <div class="login-page">
    <h2 v-if="status === 'loading'">Logging you in...</h2>
    <h2 v-if="status === 'success'"> Login successful! Redirecting...</h2>
    <h2 v-if="status === 'error'">Invalid or expired login link.</h2>
  </div>
</template>

<script setup>
import { useRouter, useRoute } from 'vue-router'
import { ref, onMounted } from 'vue'

const router = useRouter()
const route = useRoute()
const status = ref('loading')

onMounted(async () => {
  const token = route.query.token

  if (!token) {
    status.value = 'error'
    return
  }

  try {
    await $fetch('/api/login', {
      method: 'GET',
      params: { token }
    })
    status.value = 'success'
    setTimeout(() => router.push('/'), 1500)
  } catch (err) {
    console.error(err)
    status.value = 'error'
  }
})
</script>

<style scoped>
.login-page {
  text-align: center;
  padding: 2rem;
}
</style>
