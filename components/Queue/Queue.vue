<template>
    <div class="border-black rounded-lg border-4 border-b-0 rounded-b-none" style="width: 330px; height: 300px; margin-top: 4%;">
        <p class="text-center border-b-2 border-black border-b-4" style="font-weight: bold; font-size: 24px">Queue</p>
        <div class="overflow-y-auto h-full">
            <div v-for="(user, index) in queueUsers" :key="index">
            <QueueCard  v-if="index % 2 == 0" :user1="getTwoUsers(index)[0]" :user2="getTwoUsers(index)[1]" :style="{backgroundColor : changeCardColor(index)}"/>
            </div>
        </div>
    </div>
    <button v-bind:style="{backgroundColor : buttonColor}" @click="changeButton();" class = "border-4 rounded-2xl border-black content-center rounded-t-none" style="font-size: 24px; font-weight: bold; width: 100%;">
        {{ buttonStatus }}
    </button> <!--Button is unclickable due to collision with the container, margin-left:75.85 and margin-top: 37% is perfect fit-->
</template>

<script setup lang = "ts">
const emit = defineEmits(["join-queue", "leave-queue"])
const props = defineProps({
    queueUsers: {type: Array<string>, default: []},
})
const buttonStatus = ref("Join Queue");    
const buttonColor = ref("#5FE0B7");
let cardColor = "#D9D9D9";

const changeCardColor = (index:number) => {
    index = index/2
    let cardCounter = index;
    if(cardCounter % 2 === 0)
    {
        cardColor = "#D9D9D9";
    }
    else
    {
        cardColor = "#E87500";
    }
    return cardColor;
}

const getTwoUsers = (index: number) => {
    let name1: string = (index < props.queueUsers.length) ? props.queueUsers[index] : ""
    let name2: string = (index+1 < props.queueUsers.length) ? props.queueUsers[index+1] : ""
    return [name1, name2]
}

const changeButton = () => {
    if(buttonColor.value == '#5FE0B7')
    {
        emit("join-queue")
        buttonColor.value = '#FF0000';
        buttonStatus.value = 'Leave Queue';
    }
    else
    {
        emit("leave-queue")
        buttonColor.value = '#5FE0B7'
        buttonStatus.value = 'Join Queue';
    }
};
</script>