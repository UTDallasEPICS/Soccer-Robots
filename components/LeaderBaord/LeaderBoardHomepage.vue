<template>
<div class="border-black rounded-lg border-4 rounded-b-none w-[22vw] flex flex-col">
    <p class="text-center border-b-4 border-black  dark:bg-[#154734]" style="font-weight: bold; font-size: 24px;">
        Leaderboard
    </p>
    
    <div class="overflow-y-auto flex-grow p-0.5">
        <div v-for="(player, index) in topPlayers" :key="index" class="p-2 rounded text-sm" :style="{ backgroundColor: changeCardColor(index) }">
        <p><strong>#{{ index + 1 }}</strong> — {{ player.username }}</p>
        <p>🏆 Wins: {{ player.wins }}</p>
        </div>

        <p v-if="topPlayers.length === 0">Loading...</p>
    </div>
    </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import type { Player } from '@prisma/client'

const { data: playerData } = await useFetch<Player[]>('/api/leaderboard')

const topPlayers = computed(() => {
    if (!playerData.value || playerData.value.length === 0) return []
    return [...playerData.value].sort((a, b) => b.wins - a.wins).slice(0, 3)
})

const changeCardColor = (index: number) => {
    return index % 2 === 0 ? '#D9D9D9' : '#E87500'
}
</script>