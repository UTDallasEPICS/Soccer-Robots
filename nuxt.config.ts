export default defineNuxtConfig({
  runtimeConfig: {
    CHANNEL_NAME: '',
    PARENT_NAME: '',
    AUTH0_CLIENTID: '',
    AUTH0_SECRET: '',
    BASEURL: '',
    ISSUER: '',
    public: {
      CONFIRMATION_TIMER_DURATION: '',
      LOCALHOST: '',
      PORT_CLIENT_GM: '',
      PORT_WSS_CONTROLLER_CLIENT: '',
      PORT_SSE_GM: '',
    }
  },

  devtools: { enabled: true },
  modules: ['@nuxtjs/tailwindcss'],

  components: [
    { path: '~/components', pathPrefix: false } // Ensures auto-import without modifying _GlobalComponents
  ],

  compatibilityDate: '2024-10-03'
})
