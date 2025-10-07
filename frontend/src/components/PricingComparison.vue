<template>
  <div class="pricing-comparison">
    <div class="comparison-header">
      <q-card class="header-card">
        <q-card-section>
          <div class="row items-center justify-between">
            <div class="col">
              <h5 class="text-h5 q-my-none">Price Comparison Engine</h5>
              <div class="text-subtitle2 text-grey-6">
                NSE vs BSE | Cash vs Futures | Optimal Selection
              </div>
            </div>
            <div class="col-auto">
              <q-btn-group>
                <q-btn 
                  :color="viewMode === 'all' ? 'primary' : 'grey-5'"
                  label="All"
                  @click="viewMode = 'all'"
                  dense
                />
                <q-btn 
                  :color="viewMode === 'recommended' ? 'primary' : 'grey-5'"
                  label="Recommended"
                  @click="viewMode = 'recommended'"
                  dense
                />
              </q-btn-group>
            </div>
          </div>
        </q-card-section>
      </q-card>
    </div>

    <div class="comparison-filters q-mb-md">
      <q-card flat bordered>
        <q-card-section class="q-pb-none">
          <div class="row q-gutter-md items-end">
            <div class="col-3">
              <q-input
                v-model="symbolFilter"
                label="Symbol"
                placeholder="e.g., NIFTY, BANKNIFTY"
                dense
                outlined
              />
            </div>
            <div class="col-2">
              <q-select
                v-model="instrumentFilter"
                :options="instrumentOptions"
                label="Instrument"
                dense
                outlined
                clearable
              />
            </div>
            <div class="col-2">
              <q-input
                v-model.number="quantityFilter"
                label="Target Quantity"
                type="number"
                dense
                outlined
                suffix="lots"
              />
            </div>
            <div class="col-auto">
              <q-btn
                color="primary"
                icon="refresh"
                label="Refresh Prices"
                @click="refreshPrices"
                :loading="isLoading"
              />
            </div>
          </div>
        </q-card-section>
      </q-card>
    </div>

    <div class="comparison-table">
      <q-table
        :rows="filteredComparisons"
        :columns="columns"
        row-key="symbol"
        flat
        bordered
        :pagination="{ rowsPerPage: 20 }"
        :loading="isLoading"
      >
        <template #body="props">
          <q-tr :props="props" :class="{ 'recommended-row': props.row.isRecommended }">
            <q-td key="symbol" :props="props">
              <div class="symbol-cell">
                <div class="symbol-name">{{ props.row.symbol }}</div>
                <div class="instrument-type">{{ props.row.instrumentType }}</div>
                <div v-if="props.row.expiryDate" class="expiry-date">
                  {{ formatDate(props.row.expiryDate) }}
                </div>
              </div>
            </q-td>

            <q-td key="nsePrice" :props="props" class="text-right">
              <div class="price-cell">
                <div class="price">₹{{ props.row.nsePrice.toFixed(2) }}</div>
                <q-chip 
                  size="xs" 
                  color="primary" 
                  text-color="white"
                  label="NSE"
                />
              </div>
            </q-td>

            <q-td key="bsePrice" :props="props" class="text-right">
              <div class="price-cell">
                <div class="price">₹{{ props.row.bsePrice.toFixed(2) }}</div>
                <q-chip 
                  size="xs" 
                  color="orange" 
                  text-color="white"
                  label="BSE"
                />
              </div>
            </q-td>

            <q-td key="fairValue" :props="props" class="text-right">
              <div class="fair-value">₹{{ props.row.fairValue.toFixed(2) }}</div>
            </q-td>

            <q-td key="percentageDiff" :props="props" class="text-right">
              <q-chip
                :color="getDiffColor(props.row.percentageDiff)"
                :text-color="getDiffTextColor(props.row.percentageDiff)"
                :label="`${props.row.percentageDiff > 0 ? '+' : ''}${props.row.percentageDiff.toFixed(2)}%`"
              />
            </q-td>

            <q-td key="liquidity" :props="props" class="text-center">
              <div class="liquidity-info">
                <q-circular-progress
                  :value="props.row.liquidityScore * 100"
                  size="40px"
                  :thickness="0.15"
                  :color="getLiquidityColor(props.row.liquidityScore)"
                  track-color="grey-3"
                  class="q-mb-xs"
                >
                  <div class="text-caption">{{ (props.row.liquidityScore * 100).toFixed(0) }}</div>
                </q-circular-progress>
                <div class="text-caption">
                  Vol: {{ formatVolume(props.row.currentVolume) }}
                </div>
                <div class="text-caption">
                  OI: {{ formatVolume(props.row.openInterest) }}
                </div>
              </div>
            </q-td>

            <q-td key="recommendation" :props="props" class="text-center">
              <div v-if="props.row.isRecommended" class="recommendation-badge">
                <q-icon name="star" color="amber-6" size="md" />
                <div class="text-caption text-weight-medium">BEST</div>
                <div class="text-caption">{{ props.row.exchange }}</div>
              </div>
              <div v-else class="text-grey-5">
                {{ props.row.exchange }}
              </div>
            </q-td>

            <q-td key="actions" :props="props" class="text-center">
              <q-btn-group dense>
                <q-btn
                  size="sm"
                  color="positive"
                  icon="add_shopping_cart"
                  @click="createBuyOrder(props.row)"
                  :disable="!props.row.isRecommended && viewMode === 'recommended'"
                >
                  <q-tooltip>Create Buy Order</q-tooltip>
                </q-btn>
                <q-btn
                  size="sm"
                  color="negative"
                  icon="remove_shopping_cart"
                  @click="createSellOrder(props.row)"
                  :disable="!props.row.isRecommended && viewMode === 'recommended'"
                >
                  <q-tooltip>Create Sell Order</q-tooltip>
                </q-btn>
                <q-btn
                  size="sm"
                  color="info"
                  icon="info"
                  @click="showDetails(props.row)"
                >
                  <q-tooltip>View Details</q-tooltip>
                </q-btn>
              </q-btn-group>
            </q-td>
          </q-tr>
        </template>
      </q-table>
    </div>

    <!-- Details Dialog -->
    <q-dialog v-model="showDetailsDialog" maximized>
      <q-card class="details-card">
        <q-card-section class="row items-center q-pb-none">
          <h6 class="text-h6 q-my-none">Price Analysis - {{ selectedComparison?.symbol }}</h6>
          <q-space />
          <q-btn icon="close" flat round dense @click="showDetailsDialog = false" />
        </q-card-section>

        <q-card-section v-if="selectedComparison">
          <div class="row q-gutter-lg">
            <div class="col-5">
              <q-card flat bordered>
                <q-card-section>
                  <h6 class="text-h6">Price Comparison</h6>
                  <div class="price-comparison-chart">
                    <canvas ref="priceChart"></canvas>
                  </div>
                </q-card-section>
              </q-card>
            </div>
            <div class="col">
              <q-card flat bordered>
                <q-card-section>
                  <h6 class="text-h6">Liquidity Analysis</h6>
                  <div class="row q-gutter-md">
                    <div class="col">
                      <q-linear-progress
                        :value="selectedComparison.currentVolume / selectedComparison.volume30Day"
                        color="primary"
                        size="12px"
                      />
                      <div class="text-caption q-mt-xs">
                        Current Volume: {{ formatVolume(selectedComparison.currentVolume) }}
                      </div>
                      <div class="text-caption">
                        30-day Avg: {{ formatVolume(selectedComparison.volume30Day) }}
                      </div>
                    </div>
                  </div>
                  
                  <div class="q-mt-md">
                    <div class="text-subtitle2">Volume Compliance</div>
                    <div class="text-caption">
                      <q-icon 
                        :name="isVolumeCompliant(selectedComparison) ? 'check_circle' : 'error'"
                        :color="isVolumeCompliant(selectedComparison) ? 'positive' : 'negative'"
                      />
                      {{ isVolumeCompliant(selectedComparison) ? 'Within 5% limit' : 'Exceeds 5% limit' }}
                    </div>
                  </div>
                </q-card-section>
              </q-card>
            </div>
          </div>
        </q-card-section>
      </q-card>
    </q-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue';
import { PriceComparison, OrderRequest } from '../types/trading';

// Props
const props = defineProps<{
  comparisons: PriceComparison[];
}>();

// Emits
const emit = defineEmits<{
  orderCreated: [order: OrderRequest];
  refreshRequested: [];
}>();

// Reactive data
const viewMode = ref<'all' | 'recommended'>('all');
const symbolFilter = ref('');
const instrumentFilter = ref<string | null>(null);
const quantityFilter = ref<number | null>(null);
const isLoading = ref(false);
const showDetailsDialog = ref(false);
const selectedComparison = ref<PriceComparison | null>(null);
const priceChart = ref<HTMLCanvasElement | null>(null);

// Computed
const instrumentOptions = [
  { label: 'Cash', value: 'CASH' },
  { label: 'Future', value: 'FUTURE' },
  { label: 'Option', value: 'OPTION' }
];

const filteredComparisons = computed(() => {
  let filtered = props.comparisons;

  if (viewMode.value === 'recommended') {
    filtered = filtered.filter(c => c.isRecommended);
  }

  if (symbolFilter.value) {
    filtered = filtered.filter(c => 
      c.symbol.toLowerCase().includes(symbolFilter.value.toLowerCase())
    );
  }

  if (instrumentFilter.value) {
    filtered = filtered.filter(c => c.instrumentType === instrumentFilter.value);
  }

  return filtered.sort((a, b) => {
    if (a.isRecommended && !b.isRecommended) return -1;
    if (!a.isRecommended && b.isRecommended) return 1;
    return Math.abs(a.percentageDiff) - Math.abs(b.percentageDiff);
  });
});

const columns = [
  { name: 'symbol', label: 'Symbol', field: 'symbol', align: 'left' as const },
  { name: 'nsePrice', label: 'NSE Price', field: 'nsePrice', align: 'right' as const },
  { name: 'bsePrice', label: 'BSE Price', field: 'bsePrice', align: 'right' as const },
  { name: 'fairValue', label: 'Fair Value', field: 'fairValue', align: 'right' as const },
  { name: 'percentageDiff', label: 'Diff %', field: 'percentageDiff', align: 'right' as const },
  { name: 'liquidity', label: 'Liquidity', field: 'liquidity', align: 'center' as const },
  { name: 'recommendation', label: 'Recommended', field: 'recommendation', align: 'center' as const },
  { name: 'actions', label: 'Actions', field: 'actions', align: 'center' as const }
];

// Methods
const formatDate = (date: Date): string => {
  return date.toLocaleDateString('en-IN', { 
    day: '2-digit', 
    month: 'short', 
    year: '2-digit' 
  });
};

const formatVolume = (volume: number): string => {
  if (volume > 1000000) {
    return (volume / 1000000).toFixed(1) + 'M';
  } else if (volume > 1000) {
    return (volume / 1000).toFixed(1) + 'K';
  }
  return volume.toString();
};

const getDiffColor = (diff: number): string => {
  if (Math.abs(diff) < 0.1) return 'positive';
  if (Math.abs(diff) < 0.5) return 'warning';
  return 'negative';
};

const getDiffTextColor = (diff: number): string => {
  return 'white';
};

const getLiquidityColor = (score: number): string => {
  if (score > 0.8) return 'positive';
  if (score > 0.5) return 'warning';
  return 'negative';
};

const isVolumeCompliant = (comparison: PriceComparison): boolean => {
  const targetQuantity = quantityFilter.value || 1;
  const fivePercentLimit = comparison.volume30Day * 0.05;
  return targetQuantity <= fivePercentLimit;
};

const refreshPrices = async () => {
  isLoading.value = true;
  try {
    emit('refreshRequested');
    // Simulate API call delay
    await new Promise(resolve => setTimeout(resolve, 1000));
  } finally {
    isLoading.value = false;
  }
};

const createBuyOrder = (comparison: PriceComparison) => {
  const order: OrderRequest = {
    symbol: comparison.symbol,
    quantity: quantityFilter.value || 1,
    orderType: 'MARKET',
    side: 'BUY',
    instrumentType: comparison.instrumentType,
    exchange: comparison.exchange,
    productType: 'MIS'
  };
  
  emit('orderCreated', order);
};

const createSellOrder = (comparison: PriceComparison) => {
  const order: OrderRequest = {
    symbol: comparison.symbol,
    quantity: quantityFilter.value || 1,
    orderType: 'MARKET',
    side: 'SELL',
    instrumentType: comparison.instrumentType,
    exchange: comparison.exchange,
    productType: 'MIS'
  };
  
  emit('orderCreated', order);
};

const showDetails = (comparison: PriceComparison) => {
  selectedComparison.value = comparison;
  showDetailsDialog.value = true;
};

// Lifecycle
onMounted(() => {
  // Initialize any chart libraries here
});
</script>

<style scoped>
.pricing-comparison {
  width: 100%;
}

.header-card {
  background: linear-gradient(135deg, #673ab7 0%, #9c27b0 100%);
  color: white;
}

.recommended-row {
  background-color: rgba(255, 193, 7, 0.1);
}

.symbol-cell {
  min-width: 120px;
}

.symbol-name {
  font-weight: bold;
  font-size: 14px;
}

.instrument-type {
  font-size: 11px;
  color: #666;
}

.expiry-date {
  font-size: 10px;
  color: #999;
}

.price-cell {
  display: flex;
  flex-direction: column;
  align-items: flex-end;
  gap: 4px;
}

.price {
  font-weight: 500;
  font-size: 13px;
}

.fair-value {
  font-weight: bold;
  color: #1976d2;
}

.liquidity-info {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 2px;
}

.recommendation-badge {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 2px;
}

.details-card {
  min-height: 500px;
}

.price-comparison-chart {
  height: 300px;
  position: relative;
}
</style>