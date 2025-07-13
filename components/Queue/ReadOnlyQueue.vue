<template>
  <div class="readonly-queue">
    <h2>Upcoming Matches</h2>
    <ul>
      <li v-for="(username, index) in queue" :key="index">
        {{ index + 1 }}. {{ username }}
      </li>
    </ul>
  </div>
</template>

<script setup>
defineProps({
  queue: {
    type: Array,
    required: true
  }
});
</script>

<style scoped>
.readonly-queue {
  background: #fff;
  border: 1px solid #ccc;
  border-radius: 12px;
  padding: 16px;
  width: 100%;
  max-width: 400px;
  text-align: left;
}

.readonly-queue h2 {
  font-size: 20px;
  margin-bottom: 8px;
}

.readonly-queue ul {
  padding-left: 0;
  list-style: none;
}

.readonly-queue li {
  margin: 4px 0;
}
</style>
