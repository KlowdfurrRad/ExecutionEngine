import { createApp } from 'vue'
import { createPinia } from 'pinia'
import { Quasar, Notify, Dialog, Loading } from 'quasar'
import App from './App.vue'

// Import icon libraries
import '@quasar/extras/material-icons/material-icons.css'
import '@quasar/extras/material-icons-outlined/material-icons-outlined.css'
import '@quasar/extras/material-symbols-outlined/material-symbols-outlined.css'

// Import Quasar css
import 'quasar/src/css/index.sass'

const app = createApp(App)

// Install Pinia for state management
const pinia = createPinia()
app.use(pinia)

// Install Quasar
app.use(Quasar, {
  plugins: {
    Notify,
    Dialog,
    Loading
  },
  config: {
    notify: {
      position: 'top-right',
      timeout: 3000,
      textColor: 'white',
      actions: [{ icon: 'close', color: 'white' }]
    },
    loading: {
      spinnerColor: 'primary',
      spinnerSize: 140,
      backgroundColor: 'rgba(255, 255, 255, 0.9)'
    }
  }
})

// Global error handler
app.config.errorHandler = (error, instance, errorInfo) => {
  console.error('Global error:', error)
  console.error('Error info:', errorInfo)
  
  // Show user-friendly error notification
  Notify.create({
    type: 'negative',
    message: 'An unexpected error occurred. Please refresh and try again.',
    caption: 'Error in trading platform'
  })
}

// Mount the app
app.mount('#app')

// Enable performance monitoring in development
if (import.meta.env.DEV) {
  console.log('Stenoa Trading Platform - Development Mode')
}

export default app