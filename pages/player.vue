<script setup>
import { ref, onMounted } from 'vue';
import { useCookie } from '#app';

import TopNavBar from '@/components/TopNavBar.vue';
import BottomNavBar from '@/components/NavBars/BottomNavBar.vue';
import VideoStream from '@/components/VideoStream.vue';
import Scoreboard from '@/components/Scoreboard.vue';
import QueueContainer from '@/components/Queue/QueueContainer.vue';
import ConfirmMatchOverlay from '@/components/ConfirmMatch/ConfirmMatchOverlay.vue';
import AdminPanel from '@/components/AdminPanel.vue';
import LogInOverlay from '@/components/userauth/LogInOverlay.vue';

const isAuthenticated = ref(false);
const isAdmin = ref(false);

onMounted(() => {
  const cookie = useCookie('sruser');

  if (cookie.value) {
    isAuthenticated.value = true;
    isAdmin.value = cookie.value.role === 'admin';
  }
});
</script>
