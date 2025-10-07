<template>
  <div class="options-chain">
    <div class="chain-header">
      <q-card class="header-card">
        <q-card-section>
          <div class="row items-center justify-between">
            <div class="col-auto">
              <h5 class="text-h5 q-my-none">{{ underlying }} Options Chain</h5>
              <div class="text-subtitle2 text-grey-6">
                Spot: ₹{{ spotPrice.toLocaleString() }} 
                <span :class="spotChange >= 0 ? 'text-positive' : 'text-negative'">
                  ({{ spotChange >= 0 ? '+' : '' }}{{ spotChange.toFixed(2) }}%)
                </span>
              </div>
            </div>
            <div class="col-auto">
              <q-select
                v-model="selectedExpiry"
                :options="expiryOptions"
                label="Expiry"
                dense
                outlined
                @update:model-value="onExpiryChange"
              />
            </div>
          </div>
        </q-card-section>
      </q-card>
    </div>

    <div class="chain-table-container">
      <q-table
        :rows="chainData"
        :columns="columns"
        row-key="strike"
        flat
        :pagination="{ rowsPerPage: 50 }"
        class="options-table"
      >
        <template #body="props">
          <q-tr :props="props" class="chain-row">
            <!-- Call Options -->
            <q-td key="callOI" :props="props" class="text-right">
              {{ formatNumber(props.row.call?.openInterest || 0) }}
            </q-td>
            <q-td key="callVolume" :props="props" class="text-right">
              {{ formatNumber(props.row.call?.volume || 0) }}
            </q-td>
            <q-td key="callIV" :props="props" class="text-right">
              {{ (props.row.call?.impliedVolatility || 0).toFixed(2) }}%
            </q-td>
            <q-td key="callLTP" :props="props" class="text-right call-price">
              <q-btn
                flat
                dense
                :label="formatPrice(props.row.call?.marketData.price || 0)"
                @click="selectOption(props.row.call, 'CE')"
                class="price-btn"
                :class="{ 'selected': isSelected(props.row.call, 'CE') }"
              />
            </q-td>
            <q-td key="callBid" :props="props" class="text-right text-positive">
              {{ formatPrice(props.row.call?.marketData.bid || 0) }}
            </q-td>
            <q-td key="callAsk" :props="props" class="text-right text-negative">
              {{ formatPrice(props.row.call?.marketData.ask || 0) }}
            </q-td>

            <!-- Strike Price -->
            <q-td key="strike" :props="props" class="text-center strike-column">
              <div class="strike-price" :class="{ 'atm-strike': isATMStrike(props.row.strike) }">
                {{ props.row.strike }}
              </div>
            </q-td>

            <!-- Put Options -->
            <q-td key="putBid" :props="props" class="text-left text-positive">
              {{ formatPrice(props.row.put?.marketData.bid || 0) }}
            </q-td>
            <q-td key="putAsk" :props="props" class="text-left text-negative">
              {{ formatPrice(props.row.put?.marketData.ask || 0) }}
            </q-td>
            <q-td key="putLTP" :props="props" class="text-left put-price">
              <q-btn
                flat
                dense
                :label="formatPrice(props.row.put?.marketData.price || 0)"
                @click="selectOption(props.row.put, 'PE')"
                class="price-btn"
                :class="{ 'selected': isSelected(props.row.put, 'PE') }"
              />
            </q-td>
            <q-td key="putIV" :props="props" class="text-left">
              {{ (props.row.put?.impliedVolatility || 0).toFixed(2) }}%
            </q-td>
            <q-td key="putVolume" :props="props" class="text-left">
              {{ formatNumber(props.row.put?.volume || 0) }}
            </q-td>
            <q-td key="putOI" :props="props" class="text-left">
              {{ formatNumber(props.row.put?.openInterest || 0) }}
            </q-td>
          </q-tr>
        </template>
      </q-table>
    </div>

    <!-- Greeks Panel -->
    <div v-if="selectedOption" class="greeks-panel">
      <q-card>
        <q-card-section>
          <h6 class="text-h6 q-my-none">Option Greeks - {{ selectedOption.symbol }}</h6>
          <div class="row q-gutter-md q-mt-sm">
            <div class="col">
              <q-linear-progress 
                :value="Math.abs(selectedOption.greeks.delta)"
                color="primary"
                size="8px"
              />
              <div class="text-caption">Delta: {{ selectedOption.greeks.delta.toFixed(3) }}</div>
            </div>
            <div class="col">
              <q-linear-progress 
                :value="selectedOption.greeks.gamma * 100"
                color="secondary"
                size="8px"
              />
              <div class="text-caption">Gamma: {{ selectedOption.greeks.gamma.toFixed(4) }}</div>
            </div>
            <div class="col">
              <q-linear-progress 
                :value="Math.abs(selectedOption.greeks.theta) * 10"
                color="negative"
                size="8px"
              />
              <div class="text-caption">Theta: {{ selectedOption.greeks.theta.toFixed(3) }}</div>
            </div>
            <div class="col">
              <q-linear-progress 
                :value="selectedOption.greeks.vega * 10"
                color="warning"
                size="8px"
              />
              <div class="text-caption">Vega: {{ selectedOption.greeks.vega.toFixed(3) }}</div>
            </div>
          </div>
        </q-card-section>
      </q-card>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, watch } from 'vue';
import { OptionsChain, OptionContract } from '../types/trading';

interface ChainRow {
  strike: number;
  call?: OptionContract;
  put?: OptionContract;
}

// Props
const props = defineProps<{
  underlying: string;
  spotPrice: number;
  spotChange: number;
  optionsChain: OptionsChain | null;
}>();

// Emits
const emit = defineEmits<{
  optionSelected: [option: OptionContract, type: 'CE' | 'PE'];
  expiryChanged: [expiry: Date];
}>();

// Reactive data
const selectedExpiry = ref<Date | null>(null);
const selectedOption = ref<OptionContract | null>(null);

// Computed
const expiryOptions = computed(() => {
  // This would be populated from available expiry dates
  const today = new Date();
  const expiryDates = [];
  
  // Add monthly expiries for next 3 months
  for (let i = 0; i < 3; i++) {
    const expiry = new Date(today.getFullYear(), today.getMonth() + i, getLastThursday(today.getFullYear(), today.getMonth() + i));
    expiryDates.push({
      label: expiry.toLocaleDateString('en-IN', { day: '2-digit', month: 'short', year: 'numeric' }),
      value: expiry
    });
  }
  
  return expiryDates;
});

const chainData = computed(() => {
  if (!props.optionsChain) return [];
  
  const strikes = new Set<number>();
  props.optionsChain.calls.forEach(call => strikes.add(call.strike));
  props.optionsChain.puts.forEach(put => strikes.add(put.strike));
  
  const sortedStrikes = Array.from(strikes).sort((a, b) => a - b);
  
  return sortedStrikes.map(strike => {
    const call = props.optionsChain!.calls.find(c => c.strike === strike);
    const put = props.optionsChain!.puts.find(p => p.strike === strike);
    
    return { strike, call, put };
  });
});

const columns = [
  { name: 'callOI', label: 'OI', field: 'callOI', align: 'right' as const },
  { name: 'callVolume', label: 'Volume', field: 'callVolume', align: 'right' as const },
  { name: 'callIV', label: 'IV', field: 'callIV', align: 'right' as const },
  { name: 'callLTP', label: 'LTP', field: 'callLTP', align: 'right' as const },
  { name: 'callBid', label: 'Bid', field: 'callBid', align: 'right' as const },
  { name: 'callAsk', label: 'Ask', field: 'callAsk', align: 'right' as const },
  { name: 'strike', label: 'Strike', field: 'strike', align: 'center' as const },
  { name: 'putBid', label: 'Bid', field: 'putBid', align: 'left' as const },
  { name: 'putAsk', label: 'Ask', field: 'putAsk', align: 'left' as const },
  { name: 'putLTP', label: 'LTP', field: 'putLTP', align: 'left' as const },
  { name: 'putIV', label: 'IV', field: 'putIV', align: 'left' as const },
  { name: 'putVolume', label: 'Volume', field: 'putVolume', align: 'left' as const },
  { name: 'putOI', label: 'OI', field: 'putOI', align: 'left' as const }
];

// Methods
const formatPrice = (price: number): string => {
  return price.toFixed(2);
};

const formatNumber = (num: number): string => {
  if (num > 1000000) {
    return (num / 1000000).toFixed(1) + 'M';
  } else if (num > 1000) {
    return (num / 1000).toFixed(1) + 'K';
  }
  return num.toString();
};

const isATMStrike = (strike: number): boolean => {
  const diff = Math.abs(strike - props.spotPrice);
  return diff < 100; // Consider strikes within 100 points as ATM
};

const selectOption = (option: OptionContract | undefined, type: 'CE' | 'PE') => {
  if (!option) return;
  
  selectedOption.value = option;
  emit('optionSelected', option, type);
};

const isSelected = (option: OptionContract | undefined, type: 'CE' | 'PE'): boolean => {
  return selectedOption.value?.symbol === option?.symbol;
};

const onExpiryChange = (expiry: Date) => {
  emit('expiryChanged', expiry);
};

const getLastThursday = (year: number, month: number): number => {
  const lastDay = new Date(year, month + 1, 0).getDate();
  const lastDayOfMonth = new Date(year, month, lastDay);
  const dayOfWeek = lastDayOfMonth.getDay();
  const daysToSubtract = dayOfWeek >= 4 ? dayOfWeek - 4 : dayOfWeek + 3;
  return lastDay - daysToSubtract;
};

// Lifecycle
onMounted(() => {
  if (expiryOptions.value.length > 0) {
    selectedExpiry.value = expiryOptions.value[0].value;
  }
});
</script>

<style scoped>
.options-chain {
  width: 100%;
}

.chain-header {
  margin-bottom: 16px;
}

.header-card {
  background: linear-gradient(135deg, #1976d2 0%, #1565c0 100%);
  color: white;
}

.chain-table-container {
  max-height: 600px;
  overflow-y: auto;
}

.options-table {
  font-size: 12px;
}

.options-table :deep(.q-table__top) {
  display: none;
}

.chain-row:hover {
  background-color: rgba(25, 118, 210, 0.04);
}

.strike-column {
  background-color: rgba(0, 0, 0, 0.02);
  font-weight: bold;
}

.atm-strike {
  background-color: rgba(255, 193, 7, 0.2);
  border-radius: 4px;
  padding: 4px 8px;
  font-weight: bold;
}

.call-price {
  background-color: rgba(76, 175, 80, 0.05);
}

.put-price {
  background-color: rgba(244, 67, 54, 0.05);
}

.price-btn {
  min-width: 60px;
  font-weight: 500;
}

.price-btn.selected {
  background-color: rgba(25, 118, 210, 0.1);
  color: #1976d2;
  font-weight: bold;
}

.greeks-panel {
  margin-top: 16px;
}

.text-positive {
  color: #4caf50;
}

.text-negative {
  color: #f44336;
}
</style>