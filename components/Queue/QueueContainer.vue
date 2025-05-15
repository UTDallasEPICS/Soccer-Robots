<!--container for the all the queue cards.-->

<template>
<div class="w-[340px] h-full">
    <Upcoming :queue="queueUsers" />
    <LeaderBoardHomepage/>
    <Queue :queueUsers="queueUsers" @join-queue="$emit('join-queue')" @leave-queue="$emit('leave-queue')" />
  </div>
</template>

<script setup lang="ts">
//need to pass in the queue of all users here.
  import Upcoming from '@/components/UpComing.vue'

  const props = defineProps({
  queueUsers: { type: Array<string>, default: [] },
})
</script>