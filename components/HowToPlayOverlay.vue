<template>
    <div class="overlay fixed w-full h-full inset-0 bg-opacity-20 backdrop-blur-sm flex ">
      <div class="content rounded-lg p-4 bg-white border-2 border-black ">
        <div class="top-content">
          <div class="title-container">
            <p class="title">How to Play</p>
          </div>
          <div class="close-button-container">
            <p class="close-button" @click="emitClose">X</p>
          </div>
        </div>
        <div class="main-content">
          <div class="left-content">
            <div class="text-content">
              <p class="instructions">
                <br>
                <strong>Step 1:</strong> Register and Log In!<br>
                <strong>Step 2:</strong> Join the Queue!<br>
                <strong>Step 3:</strong> When your match starts, control your robot with the WASD keys. Defend the goal that matches your robot and score by getting the ball into the other color's goal.<br><br>
                <strong>Did your Robot get stuck?</strong><br>
                Someone will be by to un-stick the bots periodically. If the bots are stuck for a prolonged period of time, click "Help" and let us know!<br><br>
                <strong>Don't see the soccer robots?</strong><br>
                The arena is also our robot fighting arena! We're likely breaking things, so... please check back and try again later!<br><br>
                <strong>The robots will return to their goals and charge themselves between matches as needed!</strong>
              </p>
            </div>
          </div>
          <!-- Right Content -->
          <div class="right-content">
            <br><br>
            <img src="public/WASD.png" alt="W = Forward, A = Left, S = Right, D = Reverse" class="image">
          </div>
        </div>
      </div>
    </div>
  </template>
  
  <script setup lang="ts">
  const emit = defineEmits(['closeHowToPlayOverlay'])
  const emitClose = () => {
    emit('closeHowToPlayOverlay')
  }
  </script>
  
  <style scoped>
  .overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    border-radius: 2px;
    border-color: black;
    backdrop-filter: blur(2.5px);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 2;
  }
  
  .content {
    background-color: white;
    border-radius: 8px;
    border-color: black;
    width: 800px;
    height: 590px;
    display: flex;
    flex-direction: column;
    padding: 20px;
  }
  
  .top-content {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  
  .title-container {
    flex-grow: 1; /* Take as much space as needed to center the title */
    text-align: center;
  }
  
  .close-button-container {
    margin-left: auto; /* Push the close button to the right */
  }
  
  .title {
    font-size: 28px;
    margin: 0;
  }
  
  .close-button {
    cursor: pointer;
  }
  
  .main-content {
    display: flex;
  }
  
  .left-content {
    flex: 1;
  }
  
  .right-content {
    margin-left: auto;
  }
  
  .image {
    max-width: 400px;
    max-height: auto;
  }
  
  .text-content {
    max-width: 100%;
  }
  
  .instructions {
    word-wrap: break-word;
  }
  </style>
  