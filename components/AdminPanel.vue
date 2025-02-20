<template>
  <div class="fixed w-full h-full inset-0 bg-black bg-opacity-80 flex" style="z-index: 2">
    <div class="rounded-lg p-4 bg-white" style="width: 500px; height: auto; margin: auto;">
      <div class="w-min h-min text-black" style="margin-left: auto;">
        <!--This is the "x" mark at the top right of the admin panel, on clicking it will tell it to close the panel.-->
        <p @click="emitClose" class="cursor-pointer">X</p>
      </div>
      <p class="w-full text-center" style="font-size: 1.75em; margin-bottom: 1em;">Admin Panel</p>
      
      <!-- Buttons -->

      <div class="flex flex-col items-center gap-4">
        <!-- this button will shut down the robot, by running the Promise/Function "shutdown".-->
        <button
          @click="shutdown"
          class="bg-red-500 text-white font-bold py-2 px-4 rounded hover:bg-red-700"
        >
          Shutdown Robot
        </button>
        <!-- this button will let you change match settings-->
        <button
          @click="toggleMatchSettings"
          class="bg-blue-500 text-white font-bold py-2 px-4 rounded hover:bg-blue-700"
        >
          Match Settings
        </button>
      </div>

      <!-- Match Settings. Only show if "showMatchSettings" is enabled by clicking "match settings" -->
      <div v-if="showMatchSettings" class="mt-6">
        <p class="text-lg font-bold text-center mb-4">Update Match Settings</p>
        <div class="flex justify-center gap-4">
          <!-- Players Dropdown -->
          <div class="w-40">
            <label for="players" class="block text-sm font-medium">Players</label>
            <select
              id="players"
              v-model="numPlayers"
              class="mt-1 block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500"
            >
              <option value="1">1v1</option>
              <option value="2">2v2</option>
              <option value="3">3v3</option>
              <option value="4">4v4</option>
            </select>
          </div>

          <!-- Match Time Input -->
          <div class="w-40">
            <label for="matchTime" class="block text-sm font-medium">Match Time (min:sec)</label>
            <input
              id="matchTime"
              type="text"
              v-model="matchTime"
              placeholder="MM:SS"
              class="mt-1 block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500"
            />
          </div>
        </div>

        <!-- Submit Button -->
        <div class="flex justify-center mt-4">
          <button
            @click="updateMatchSettings"
            class="bg-green-500 text-white font-bold py-2 px-4 rounded hover:bg-green-700"
          >
            Save Settings
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
// Function to get cookie value by name
const getCookie = (name: string): string | null => {
  // Document is a property of the current page/website. We can get the cookies from there by calling "document.cookie"
  const cookies = document.cookie.split('; ');
  // Find the cookie for the current user with the name that's passed in.
  const cookie = cookies.find((c) => c.startsWith(`${name}=`));
  // If the cookie was found, decode it and return.
  return cookie ? decodeURIComponent(cookie.split('=')[1]) : null;
};

const emit = defineEmits(['closeAdminPanel']);
const emitClose = () => {
  emit('closeAdminPanel');
};

// State for Match Settings
const showMatchSettings = ref(false);
const numPlayers = ref(1); // Default to 1v1 (1 player per team)
const matchTime = ref('01:00'); // Default match time 1 minute

// Functions
const shutdown = async () => {
  console.log('Shutdown clicked!');
  const sruser = getCookie('sruser');
  if (!sruser) {
    alert('User information not found. Please log in again.');
    return;
  }

  const user = JSON.parse(sruser);
  const role = user.role;

  console.log('Sending request to the backend');
  const response = await fetch('http://localhost:3001/shutdownrobot', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'role': role,
    },
  });

  if (response.status === 403) {
    alert('Unauthorized: You do not have admin privileges.');
    return;
  }

  if (!response.ok) {
    throw new Error(`Failed to shutdown the robot: ${response.statusText}`);
  }

  const data = await response.json();
  console.log('Backend response:', data);
  alert('Shutdown command sent to the robot!');
};

// Toggle Match Settings View
const toggleMatchSettings = () => {
  showMatchSettings.value = !showMatchSettings.value;
};

// Save Match Settings
const updateMatchSettings = async () => {
  const sruser = getCookie('sruser');
  if (!sruser) {
    alert('User information not found. Please log in again.');
    return;
  }

  const user = JSON.parse(sruser);
  const role = user.role;

  if (!matchTime.value.match(/^\d{1,2}:\d{2}$/)) {
    alert('Invalid match time. Please use the format MM:SS.');
    return;
  }

  const [minutes, seconds] = matchTime.value.split(':').map(Number);
  if (minutes < 1 || minutes > 8 || seconds < 0 || seconds >= 60) {
    alert('Match time must be between 1 and 8 minutes, with valid seconds.');
    return;
  }

  console.log('Sending match settings to the backend');
  const response = await fetch('http://localhost:3001/editMatchSettings', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'role': role,
    },
    body: JSON.stringify({
      numPlayers: numPlayers.value,
      matchTime: minutes * 60 + seconds,
    }),
  });

  if (response.status === 403) {
    alert('Unauthorized: You do not have admin privileges.');
    return;
  }

  if (!response.ok) {
    throw new Error(`Failed to update match settings: ${response.statusText}`);
  }

  const data = await response.json();
  console.log('Backend response:', data);

  showMatchSettings.value = false;
  alert('Match settings updated successfully!');
};
</script>