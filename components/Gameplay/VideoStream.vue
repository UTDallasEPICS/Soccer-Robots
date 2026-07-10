<template>
  <div class="video-container">
    <!-- Twitch Embed -->
    <iframe
      v-if="streamType === 'twitch'"
      :src="twitchEmbedUrl"
      frameborder="0"
      allowfullscreen
      class="stream-frame"
    ></iframe>

    <!-- Janus (WebRTC) -->
    <div v-else-if="streamType === 'janus'" class="stream-frame">
      <video
        id="janus-video"
        autoplay
        muted
        playsinline
        controls
        ref="janusVideo"
      ></video>
    </div>

    <!-- MJPEG Stream -->
    <div v-else-if="streamType === 'mjpeg'" class="stream-frame">
      <img
        :src="mjpegUrl"
        class="mjpeg-frame"
        alt="MJPEG Stream"
      />
    </div>

    <!-- Fallback -->
    <div v-else class="error-msg">
      Unknown stream type.
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue';

const props = defineProps({
  streamType: {
    type: String,
    required: true,
    // Add mjpeg as allowed:
    validator: (value) => ['twitch', 'janus', 'mjpeg'].includes(value),
  },
});

// Reference to Janus <video> tag
const janusVideo = ref(null);

// Hard-coded for now, or pass from parent:
const mjpegUrl = `http://172.20.10.11:8000/stream.mjpg`;

// Twitch embed
const twitchEmbedUrl = 'https://player.twitch.tv/?channel=Soccer_Robots&parent=localhost';

onMounted(() => {
  if (props.streamType === 'janus') {
    console.log('Initialize Janus WebRTC');
  }
});
</script>

<style scoped>
.video-container {
  width: 100%;
  max-width: 960px;
  margin: auto;
  padding: 10px;
}

.stream-frame {
  width: 100%;
  height: 540px;
}

.mjpeg-frame {
  width: 100%;
  height: 100%;
  object-fit: contain;
}

.error-msg {
  color: red;
  font-weight: bold;
  text-align: center;
}
</style>
