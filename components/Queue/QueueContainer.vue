<template>
  <div class="w-340 h-full mt-[-12px]">
    <Upcoming :queue="queueUsers" />
    <LeaderBoardHomepage/>
    <Queue :queueUsers="queueUsers" @join-queue="$emit('join-queue')" @leave-queue="$emit('leave-queue')" />
  </div>
</template>

<script setup lang="ts">
  import Upcoming from '@/components/UpComing.vue'

  const props = defineProps({
  queueUsers: { type: Array<string>, default: [] },
})
</script>