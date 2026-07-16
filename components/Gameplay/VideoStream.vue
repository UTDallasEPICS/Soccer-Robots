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

    <!-- MJPEG Stream -->
    <iframe
  v-else-if="streamType === 'mjpeg'"
  :src="mjpegPageUrl"
  frameborder="0"
  class="stream-frame"
></iframe>

    <!-- Janus Embed -->
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
    validator: (value) =>
      ['twitch', 'janus', 'mjpeg'].includes(value),
  },
});

const janusVideo = ref(null);
const mjpegPageUrl = 'http://10.159.181.248:8000/index.html';
const twitchEmbedUrl =
  'https://player.twitch.tv/?channel=Soccer_Robots&parent=localhost';



onMounted(() => {
  if (props.streamType === 'janus') {
    console.log('Janus stream to be initialized');
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
  display: block;
  width: 100%;
  height: 540px;
  object-fit: contain;
  border: 0;
}

#janus-video {
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