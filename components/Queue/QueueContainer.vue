<template>
<div class="w-340 h-full mt-[-12px]">
  <UpComing/>
  <Queue :queueUsers="queueUsers" @join-queue="$emit('join-queue')" @leave-queue="$emit('leave-queue')"/>
</div>
</template>

<script setup lang="ts">
const props = defineProps({
  queueUsers: {type: Array<string>, default: []},
})
</script>