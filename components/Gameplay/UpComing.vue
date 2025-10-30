<!--Component to show the next upcoming match. Currently just set to arbitrary names.-->
<template>

<div class="border rounded-lg p-2.5 text-center border-4 border-black dark:bg-[#C0C0C0] w-[93.125%] h-[9vh] my-2">
    <p style="font-style: italic">Up Next </p>
    <div class="flex place-content-evenly flex-no-wrap" style="font-style: italic; padding-right:0px">
        <p class="basis-0 grow truncate text-center"><strong>{{user1}}</strong></p>
        <p class="text-center w-min"><strong>vs</strong></p>
        <p class="basis-0 truncate grow text-center"><strong>{{user2}}</strong></p>
    </div>
</div>
</template>

<script setup lang="ts"> 
// Sets up random placeholder usernames.
const props = defineProps<{ queue: string[] }>()

const user1 = computed(() => props.queue[0] || "TBD")
const user2 = computed(() => props.queue[1] || "TBD")

</script>

