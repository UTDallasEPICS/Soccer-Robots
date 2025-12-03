<!-- template for designing the queue.-->
<template>
    <div v-if="isLoggedIn" class="border-black rounded-lg border-4 border-b-0 rounded-b-none" style="width: 93.125%; height: 30.5vh; margin-top: 4%;">
        <p class="text-center border-b-2 border-black border-b-4 dark:bg-[#154734]" style="font-weight: bold; font-size: 24px">Queue</p>
        <div class="overflow-y-auto h-full">
            <!--for every upcoming match, show the queue card.-->
            <div v-for="(user, index) in queueUsers" :key="index">
                <!-- only show the ueue card if on an even index. Next, pass in the first user with getTwoUsers(index)[0] as the first prop, and as the 2nd prop pass
                 in the second user with getTwoUsers(index)[1]-->
                <QueueCard  v-if="index % 2 == 0" :user1="getTwoUsers(index)[0]" :user2="getTwoUsers(index)[1]" :style="{backgroundColor : changeCardColor(index)}"/>
            </div>
        </div>
    </div>

    <!--This is the button to join the queue, text in the button is buttonStatus, depending on if user's in queue or not. Call changeButton function on click.
    Also, change button color based on status.-->
    <button v-bind:style="{backgroundColor : buttonColor}" @click="changeButton();" class = "border-4 rounded-2xl border-black content-center rounded-t-none" style="font-size: 24px; font-weight: bold; width: 93.125%;">
        {{ buttonStatus }}
    </button> <!--Button is unclickable due to collision with the container, margin-left:75.85 and margin-top: 37% is perfect fit-->
</template>

<script setup lang = "ts">
const sruser = useCookie('sruser');
const isLoggedIn = computed(() => !!sruser.value)

const emit = defineEmits(["join-queue", "leave-queue"])
const props = defineProps({
    queueUsers: {type: Array<string>, default: []},
})
const buttonStatus = ref("Join Queue");    
//This is a teal color
const buttonColor = ref("#5FE0B7");
//Color of the queue card, it is gray
let cardColor = "#D9D9D9";

//function to change color of the queue card
const changeCardColor = (index:number) => {
    //this is the index of the queue card instead of the index of the users.
    let cardCounter = index / 2;
    //every other queue card has a different color. Even ones are gray, odd ones are orange
    if(cardCounter % 2 === 0)
    {
        //gray
        cardColor = "#D9D9D9";
    }
    else
    {
        //orange
        cardColor = "#E87500";
    }
    return cardColor;
}

//Function to return the two users who are going to be competing in a match based on an index
const getTwoUsers = (index: number) => {
    // First if there is actually a user at that index, get it from the array queueUsers.
    let name1: string = (index < props.queueUsers.length) ? props.queueUsers[index] : ""
    let name2: string = (index+1 < props.queueUsers.length) ? props.queueUsers[index+1] : ""
    return [name1, name2]
}

//changes the queue button based on if the user is in the queue.
const changeButton = () => {
    //if button is teal and it's pressed, have user join queue and change color to read.
    if(buttonColor.value == '#5FE0B7')
    {
        console.log("🟢 Emitting join-queue event")
        emit("join-queue")
        buttonColor.value = '#FF0000';
        buttonStatus.value = 'Leave Queue';
    }
    //if button is red (in queue) and its pressed, have user leave queue and change color to teal.
    else
    {

        console.log("🔴 Emitting leave-queue event")
        emit("leave-queue")
        buttonColor.value = '#5FE0B7'
        buttonStatus.value = 'Join Queue';
    }
};
</script>