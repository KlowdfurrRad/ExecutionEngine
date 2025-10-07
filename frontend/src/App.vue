<template>
  <q-app>
    <TradingDashboard />
  </q-app>
</template>

<script setup lang="ts">
import { onMounted } from 'vue'
import TradingDashboard from './components/TradingDashboard.vue'

onMounted(() => {
  // Initialize app-level configurations
  console.log('Stenoa Trading Platform initialized')
})
</script>

<style>
#app {
  font-family: 'Roboto', -apple-system, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  margin: 0;
  padding: 0;
}

/* Global trading platform styles */
.q-table tbody td {
  font-size: 12px;
}

.q-table thead th {
  font-size: 11px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

/* Custom scrollbar for webkit browsers */
::-webkit-scrollbar {
  width: 6px;
  height: 6px;
}

::-webkit-scrollbar-track {
  background: #f1f1f1;
}

::-webkit-scrollbar-thumb {
  background: #c1c1c1;
  border-radius: 3px;
}

::-webkit-scrollbar-thumb:hover {
  background: #a8a8a8;
}

/* Trading color scheme */
.text-positive {
  color: #4caf50 !important;
}

.text-negative {
  color: #f44336 !important;
}

.bg-positive {
  background-color: #4caf50 !important;
}

.bg-negative {
  background-color: #f44336 !important;
}

/* Loading animation */
.loading-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(255, 255, 255, 0.9);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

/* Price blinking animation */
@keyframes price-up {
  0% { background-color: transparent; }
  50% { background-color: rgba(76, 175, 80, 0.2); }
  100% { background-color: transparent; }
}

@keyframes price-down {
  0% { background-color: transparent; }
  50% { background-color: rgba(244, 67, 54, 0.2); }
  100% { background-color: transparent; }
}

.price-blink-up {
  animation: price-up 1s ease-in-out;
}

.price-blink-down {
  animation: price-down 1s ease-in-out;
}
</style>