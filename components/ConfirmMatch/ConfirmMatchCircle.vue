<!--Simple stuff to draw a circular timer, where once if the timer reaches 0 and both user's have accepted, it starts.-->
<template>
<svg :height="radius * 2" :width="radius * 2">
    <g :style="transformation">
        <!--This is the blue stroke, stays the same and is behind the orange stroke-->
        <!--NOTE: for the cx and cy, i have it start at 0 and then we transform it in the "transformation"
        method. This is because that way, we can translate first then rotate in place, unlike before-->
        <circle
            stroke="#154734"
            fill="transparent"
            :style="{ strokeDashoffset }"
            :stroke-width="stroke"
            :r="normalizedRadius"
            :cx="0"
            :cy="0"
        />
        <!--This is the orange stroke, decreases to reveal time left-->
        <circle
            stroke="#e87500"
            fill="transparent"
            :stroke-dasharray="circumference + ' ' + circumference"
            :style="{ strokeDashoffset }"
            :stroke-width="stroke-10"
            :r="normalizedRadius"
            :cx="0"
            :cy="0"
        />
    </g>
</svg>
</template>

<script setup lang="ts">
//Meaning that we emit this circle when the emit "confirm response" fires
const emit = defineEmits(['confirm-response'])
//In our other vue file ConfirmMatchOverlay is where we define these numbers.
const props = defineProps({
    radius: {type: Number, required: true},
    progress: {type: Number, required: true},
    stroke: {type: Number, required: true}
})

const normalizedRadius: number = props.radius - props.stroke * 2
const circumference = normalizedRadius * 2 * Math.PI

// As progress increases, strokeDashoffset decreases, revealing more of the orange stroke.

// Computed function is called each time its dependencies (so props.progress) changes

//Here, when stroke dash offset is 0, the entire circle can be seen. When it's equal to the
// circumference, the orange circle can't be seen. Over time it increases ans progress decreases.
const strokeDashoffset = computed(() => {
    return circumference - props.progress / 100 * circumference
})

// Transforms the circle to its proper position. First we translate it to its proper position, then rotate it so that
// the orange bar starts going away from the top.
const transformation = computed(() => {
    return { transform: 'translate(' + props.radius + 'px, ' + props.radius + 'px) rotate(' + 0.75 + 'turn)' }
})
</script>