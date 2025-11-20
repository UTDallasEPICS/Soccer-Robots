<template>
  <div class="guest-page">
    <!-- Top Navigation Bar -->
    <TopNavBar
      @toggle-help="showHelp = !showHelp"
      @toggle-about="showAbout = !showAbout"
      @toggle-how="showHowToPlay = !showHowToPlay"
    />

    <!-- Scoreboard -->
    <div class="scoreboard-container">
      <Scoreboard />
    </div>

    <!-- Twitch Stream -->
    <div class="stream-container">
      <VideoStream streamType="twitch" />
    </div>

    <!-- Upcoming Matches -->
    <div class="queue-container">
      <ReadOnlyQueue :queue="queue" />
    </div>

    <!-- Overlays -->
    <HelpOverlay v-if="showHelp" @closeHelpOverlay="showHelp = false" />
    <AboutUsOverlay v-if="showAbout" @closeAboutUsOverlay="showAbout = false" />
    <HowToPlayOverlay v-if="showHowToPlay" @closeHowToPlayOverlay="showHowToPlay = false" />
  </div>
</template>
<!-- Import the compnents needed -->
<script setup>
import { ref, onMounted } from 'vue';
import { useRuntimeConfig } from '#app';

import TopNavBar from '@/components/NavBars/TopNavBar.vue';
import Scoreboard from '@/components/Gameplay/Scoreboard.vue';
import VideoStream from '@/components/Gameplay/VideoStream.vue';
import ReadOnlyQueue from '@/components/Queue/ReadOnlyQueue.vue';

import HelpOverlay from '@/components/Popups/HelpOverlay.vue';
import AboutUsOverlay from '@/components/Popups/AboutUsOverlay.vue';
import HowToPlayOverlay from '@/components/Popups/HowToPlayOverlay.vue';

const showHelp = ref(false);
const showAbout = ref(false);
const showHowToPlay = ref(false);

const queue = ref([]);

onMounted(() => {
  if (process.client) {
    const config = useRuntimeConfig();
    const sse = new EventSource(`http://${config.public.LOCALHOST}:${config.public.PORT_SSE_GM}/sse-info`);
    sse.addEventListener("message", (message) => {
      const data = JSON.parse(message.data);
      if (data.type === "UPDATE_QUEUE") {
        queue.value = data.payload;
      }
    });
  }
});
</script>
<!-- styles for the guest page, fixed centering issues and whatnot -->
<style scoped>
.guest-page {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 0;
  gap: 16px;
}

.scoreboard-container {
  width: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  margin-top: 16px;
}

.scoreboard-container :deep(*) {
  margin-left: auto;
  margin-right: auto;
}

.stream-container {
  width: 100%;
  display: flex;
  justify-content: center;
  padding-bottom: 20px;
}

.queue-container {
  width: 100%;
  display: flex;
  justify-content: center;
  padding-bottom: 40px;
}

iframe, video {
  width: 100%;
  max-width: 960px;
  aspect-ratio: 16 / 9;
  border: none;
}
</style>