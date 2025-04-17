<template>
<svg :height="radius * 2" :width="radius * 2">
    <g :style="transformation">
        <circle
            stroke="#154734"
            fill="transparent"
            :style="{ strokeDashoffset }"
            :stroke-width="stroke"
            :r="normalizedRadius"
            :cx="radius"
            :cy="radius"
        />
        <circle
            stroke="#e87500"
            fill="transparent"
            :stroke-dasharray="circumference + ' ' + circumference"
            :style="{ strokeDashoffset }"
            :stroke-width="stroke-10"
            :r="normalizedRadius"
            :cx="radius"
            :cy="radius"
        />
    </g>
</svg>
</template>

<script setup lang="ts">
const emit = defineEmits(['confirm-response'])
const props = defineProps({
    radius: {type: Number, required: true},
    progress: {type: Number, required: true},
    stroke: {type: Number, required: true}
})

const normalizedRadius: number = props.radius - props.stroke * 2
const circumference = normalizedRadius * 2 * Math.PI

const strokeDashoffset = computed(() => {
    return circumference - props.progress / 100 * circumference
})

const transformation = computed(() => {
    return { transform: 'rotate(' + 0.25 + 'turn) scaleY(' + -1 + ')'}
})
</script>