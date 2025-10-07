<template>
  <div class="trading-dashboard">
    <div class="dashboard-header">
      <q-toolbar class="bg-primary text-white">
        <q-toolbar-title>
          <div class="row items-center">
            <q-icon name="trending_up" size="md" class="q-mr-sm" />
            <span>Stenoa Trading Platform</span>
          </div>
        </q-toolbar-title>
        
        <div class="row items-center q-gutter-md">
          <div class="market-status">
            <q-chip 
              :color="marketStatus.isOpen ? 'positive' : 'negative'"
              :icon="marketStatus.isOpen ? 'circle' : 'pause_circle'"
              :label="marketStatus.isOpen ? 'Market Open' : 'Market Closed'"
              text-color="white"
            />
          </div>
          <div class="portfolio-summary">
            <div class="text-caption">Portfolio Value</div>
            <div class="text-h6">₹{{ formatCurrency(portfolioValue) }}</div>
          </div>
          <q-btn
            flat
            icon="refresh"
            @click="refreshData"
            :loading="isLoading"
          >
            <q-tooltip>Refresh Market Data</q-tooltip>
          </q-btn>
        </div>
      </q-toolbar>
    </div>

    <div class="dashboard-content">
      <q-splitter
        v-model="splitterModel"
        style="height: calc(100vh - 60px)"
      >
        <!-- Left Panel - Options Chain -->
        <template #before>
          <div class="left-panel q-pa-md">
            <div class="panel-header q-mb-md">
              <div class="row items-center justify-between">
                <h6 class="text-h6 q-my-none">Options Trading</h6>
                <q-select
                  v-model="selectedUnderlying"
                  :options="underlyingOptions"
                  label="Select Underlying"
                  style="min-width: 150px"
                  dense
                  outlined
                  @update:model-value="onUnderlyingChange"
                />
              </div>
            </div>
            
            <OptionsChain
              :underlying="selectedUnderlying"
              :spot-price="currentSpotPrice"
              :spot-change="currentSpotChange"
              :options-chain="optionsChainData"
              @option-selected="onOptionSelected"
              @expiry-changed="onExpiryChanged"
            />
            
            <!-- Strategy Builder Panel -->
            <div v-if="selectedOptions.length > 0" class="strategy-panel q-mt-md">
              <q-card>
                <q-card-section>
                  <h6 class="text-h6 q-my-none">Strategy Builder</h6>
                  <div class="strategy-legs q-mt-sm">
                    <div 
                      v-for="(leg, index) in selectedOptions" 
                      :key="index"
                      class="strategy-leg q-mb-sm"
                    >
                      <q-card flat bordered>
                        <q-card-section class="q-pa-sm">
                          <div class="row items-center justify-between">
                            <div class="col">
                              <div class="text-weight-medium">{{ leg.option.symbol }}</div>
                              <div class="text-caption text-grey-6">
                                Strike: {{ leg.option.strike }} | Premium: ₹{{ leg.option.marketData.price }}
                              </div>
                            </div>
                            <div class="col-auto">
                              <q-btn-group dense>
                                <q-btn
                                  size="sm"
                                  :color="leg.side === 'BUY' ? 'positive' : 'grey-5'"
                                  label="BUY"
                                  @click="updateLegSide(index, 'BUY')"
                                />
                                <q-btn
                                  size="sm"
                                  :color="leg.side === 'SELL' ? 'negative' : 'grey-5'"
                                  label="SELL"
                                  @click="updateLegSide(index, 'SELL')"
                                />
                              </q-btn-group>
                            </div>
                            <div class="col-auto">
                              <q-input
                                v-model.number="leg.quantity"
                                type="number"
                                dense
                                style="width: 80px"
                                suffix="lots"
                                @update:model-value="updateStrategy"
                              />
                            </div>
                            <div class="col-auto">
                              <q-btn
                                flat
                                round
                                icon="close"
                                size="sm"
                                @click="removeLeg(index)"
                              />
                            </div>
                          </div>
                        </q-card-section>
                      </q-card>
                    </div>
                  </div>
                  
                  <div class="strategy-summary q-mt-md">
                    <div class="row q-gutter-md">
                      <div class="col">
                        <q-linear-progress
                          :value="Math.abs(strategyMetrics.totalPremium) / 50000"
                          color="primary"
                          size="8px"
                        />
                        <div class="text-caption q-mt-xs">
                          Net Premium: ₹{{ strategyMetrics.totalPremium.toFixed(2) }}
                        </div>
                      </div>
                      <div class="col">
                        <q-linear-progress
                          :value="strategyMetrics.marginRequired / 100000"
                          color="warning"
                          size="8px"
                        />
                        <div class="text-caption q-mt-xs">
                          Margin: ₹{{ formatCurrency(strategyMetrics.marginRequired) }}
                        </div>
                      </div>
                    </div>
                    
                    <div class="strategy-actions q-mt-md">
                      <q-btn
                        color="primary"
                        label="Execute Strategy"
                        icon="play_arrow"
                        @click="executeStrategy"
                        :loading="isExecuting"
                        :disable="selectedOptions.length === 0"
                      />
                      <q-btn
                        flat
                        label="Clear All"
                        @click="clearStrategy"
                        class="q-ml-sm"
                      />
                    </div>
                  </div>
                </q-card-section>
              </q-card>
            </div>
          </div>
        </template>

        <!-- Right Panel - Pricing Comparison -->
        <template #after>
          <div class="right-panel q-pa-md">
            <div class="panel-header q-mb-md">
              <h6 class="text-h6 q-my-none">Price Analysis & Comparison</h6>
            </div>
            
            <PricingComparison
              :comparisons="priceComparisons"
              @order-created="onOrderCreated"
              @refresh-requested="refreshPricing"
            />
            
            <!-- Quick Order Panel -->
            <div class="quick-order-panel q-mt-md">
              <q-card>
                <q-card-section>
                  <h6 class="text-h6 q-my-none">Quick Order Entry</h6>
                  <div class="row q-gutter-md q-mt-sm">
                    <div class="col-3">
                      <q-select
                        v-model="quickOrder.symbol"
                        :options="availableSymbols"
                        label="Symbol"
                        dense
                        outlined
                      />
                    </div>
                    <div class="col-2">
                      <q-input
                        v-model.number="quickOrder.quantity"
                        label="Quantity"
                        type="number"
                        dense
                        outlined
                        suffix="lots"
                      />
                    </div>
                    <div class="col-2">
                      <q-select
                        v-model="quickOrder.orderType"
                        :options="orderTypeOptions"
                        label="Order Type"
                        dense
                        outlined
                      />
                    </div>
                    <div class="col-2">
                      <q-input
                        v-model.number="quickOrder.price"
                        label="Price"
                        type="number"
                        dense
                        outlined
                        prefix="₹"
                        :disable="quickOrder.orderType === 'MARKET'"
                      />
                    </div>
                    <div class="col-auto">
                      <q-btn-group>
                        <q-btn
                          color="positive"
                          label="BUY"
                          @click="placeQuickOrder('BUY')"
                          :loading="isPlacingOrder"
                        />
                        <q-btn
                          color="negative"
                          label="SELL"
                          @click="placeQuickOrder('SELL')"
                          :loading="isPlacingOrder"
                        />
                      </q-btn-group>
                    </div>
                  </div>
                </q-card-section>
              </q-card>
            </div>

            <!-- Recent Orders -->
            <div class="recent-orders q-mt-md">
              <q-card>
                <q-card-section>
                  <h6 class="text-h6 q-my-none">Recent Orders</h6>
                  <q-list>
                    <q-item
                      v-for="order in recentOrders"
                      :key="order.id"
                      class="order-item"
                    >
                      <q-item-section>
                        <div class="row items-center justify-between">
                          <div class="col">
                            <div class="text-weight-medium">{{ order.request.symbol }}</div>
                            <div class="text-caption">
                              {{ order.request.side }} {{ order.request.quantity }} @ ₹{{ order.executedPrice || order.request.price || 'Market' }}
                            </div>
                          </div>
                          <div class="col-auto">
                            <q-chip
                              :color="getOrderStatusColor(order.status)"
                              :label="order.status"
                              size="sm"
                              text-color="white"
                            />
                          </div>
                        </div>
                      </q-item-section>
                    </q-item>
                  </q-list>
                </q-card-section>
              </q-card>
            </div>
          </div>
        </template>
      </q-splitter>
    </div>

    <!-- Order Confirmation Dialog -->
    <q-dialog v-model="showOrderConfirm" persistent>
      <q-card style="min-width: 350px">
        <q-card-section>
          <div class="text-h6">Confirm Order</div>
        </q-card-section>

        <q-card-section v-if="pendingOrder">
          <div class="order-details">
            <div class="row justify-between q-mb-sm">
              <span>Symbol:</span>
              <span class="text-weight-bold">{{ pendingOrder.symbol }}</span>
            </div>
            <div class="row justify-between q-mb-sm">
              <span>Side:</span>
              <span :class="pendingOrder.side === 'BUY' ? 'text-positive' : 'text-negative'">
                {{ pendingOrder.side }}
              </span>
            </div>
            <div class="row justify-between q-mb-sm">
              <span>Quantity:</span>
              <span>{{ pendingOrder.quantity }} lots</span>
            </div>
            <div class="row justify-between q-mb-sm">
              <span>Price:</span>
              <span>{{ pendingOrder.orderType === 'MARKET' ? 'Market Price' : '₹' + pendingOrder.price }}</span>
            </div>
            <div class="row justify-between">
              <span>Estimated Value:</span>
              <span class="text-weight-bold">₹{{ calculateOrderValue(pendingOrder) }}</span>
            </div>
          </div>
        </q-card-section>

        <q-card-actions align="right">
          <q-btn flat label="Cancel" @click="showOrderConfirm = false" />
          <q-btn 
            color="primary" 
            label="Confirm" 
            @click="confirmOrder"
            :loading="isPlacingOrder"
          />
        </q-card-actions>
      </q-card>
    </q-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, reactive } from 'vue';
import OptionsChain from './OptionsChain.vue';
import PricingComparison from './PricingComparison.vue';
import { 
  OptionsChain as OptionsChainType, 
  OptionContract, 
  PriceComparison, 
  OrderRequest, 
  Order,
  TradingStrategy,
  StrategyLeg
} from '../types/trading';

// Reactive data
const splitterModel = ref(60);
const selectedUnderlying = ref('NIFTY');
const isLoading = ref(false);
const isExecuting = ref(false);
const isPlacingOrder = ref(false);
const showOrderConfirm = ref(false);

// Market data
const marketStatus = reactive({
  isOpen: true,
  nextSession: '09:15 AM'
});

const portfolioValue = ref(5000000); // ₹50 lakh
const currentSpotPrice = ref(19850.50);
const currentSpotChange = ref(1.25);

// Options and strategy data
const optionsChainData = ref<OptionsChainType | null>(null);
const selectedOptions = ref<StrategyLeg[]>([]);
const priceComparisons = ref<PriceComparison[]>([]);
const recentOrders = ref<Order[]>([]);

// Quick order form
const quickOrder = reactive({
  symbol: 'NIFTY50',
  quantity: 1,
  orderType: 'MARKET',
  price: null as number | null
});

const pendingOrder = ref<OrderRequest | null>(null);

// Options and computed
const underlyingOptions = ['NIFTY', 'BANKNIFTY', 'FINNIFTY', 'SENSEX'];
const availableSymbols = ['NIFTY50', 'BANKNIFTY', 'FINNIFTY', 'SENSEX'];
const orderTypeOptions = [
  { label: 'Market', value: 'MARKET' },
  { label: 'Limit', value: 'LIMIT' }
];

const strategyMetrics = computed(() => {
  let totalPremium = 0;
  let marginRequired = 0;
  
  selectedOptions.value.forEach(leg => {
    const premium = leg.option.marketData.price * leg.quantity;
    if (leg.side === 'BUY') {
      totalPremium -= premium;
      marginRequired += premium;
    } else {
      totalPremium += premium;
      marginRequired += premium * 0.2; // 20% margin for selling
    }
  });
  
  return { totalPremium, marginRequired };
});

// Methods
const formatCurrency = (amount: number): string => {
  if (amount > 10000000) {
    return (amount / 10000000).toFixed(1) + ' Cr';
  } else if (amount > 100000) {
    return (amount / 100000).toFixed(1) + ' L';
  }
  return amount.toLocaleString();
};

const onUnderlyingChange = (underlying: string) => {
  selectedUnderlying.value = underlying;
  loadOptionsChain();
};

const onOptionSelected = (option: OptionContract, type: 'CE' | 'PE') => {
  const newLeg: StrategyLeg = {
    id: Date.now().toString(),
    contract: option,
    quantity: 1,
    side: 'BUY',
    price: option.marketData.price
  };
  
  selectedOptions.value.push(newLeg);
  updateStrategy();
};

const onExpiryChanged = (expiry: Date) => {
  loadOptionsChain();
};

const updateLegSide = (index: number, side: 'BUY' | 'SELL') => {
  if (selectedOptions.value[index]) {
    selectedOptions.value[index].side = side;
    updateStrategy();
  }
};

const removeLeg = (index: number) => {
  selectedOptions.value.splice(index, 1);
  updateStrategy();
};

const updateStrategy = () => {
  // Recalculate strategy metrics (handled by computed property)
  console.log('Strategy updated:', selectedOptions.value);
};

const clearStrategy = () => {
  selectedOptions.value = [];
};

const executeStrategy = async () => {
  if (selectedOptions.value.length === 0) return;
  
  isExecuting.value = true;
  try {
    // Convert strategy legs to individual orders
    for (const leg of selectedOptions.value) {
      const order: OrderRequest = {
        symbol: leg.contract.symbol,
        quantity: leg.quantity,
        orderType: 'MARKET',
        side: leg.side,
        instrumentType: 'OPTION',
        exchange: 'NSE',
        productType: 'MIS'
      };
      
      await submitOrder(order);
    }
    
    clearStrategy();
  } finally {
    isExecuting.value = false;
  }
};

const onOrderCreated = (order: OrderRequest) => {
  pendingOrder.value = order;
  showOrderConfirm.value = true;
};

const placeQuickOrder = (side: 'BUY' | 'SELL') => {
  if (!quickOrder.symbol || !quickOrder.quantity) return;
  
  const order: OrderRequest = {
    symbol: quickOrder.symbol,
    quantity: quickOrder.quantity,
    orderType: quickOrder.orderType as 'MARKET' | 'LIMIT',
    side,
    price: quickOrder.orderType === 'LIMIT' ? quickOrder.price : undefined,
    instrumentType: 'CASH',
    exchange: 'NSE',
    productType: 'MIS'
  };
  
  pendingOrder.value = order;
  showOrderConfirm.value = true;
};

const confirmOrder = async () => {
  if (!pendingOrder.value) return;
  
  isPlacingOrder.value = true;
  try {
    await submitOrder(pendingOrder.value);
    showOrderConfirm.value = false;
    pendingOrder.value = null;
  } finally {
    isPlacingOrder.value = false;
  }
};

const submitOrder = async (order: OrderRequest): Promise<void> => {
  // Simulate API call
  await new Promise(resolve => setTimeout(resolve, 1000));
  
  const newOrder: Order = {
    id: Date.now().toString(),
    request: order,
    status: 'PENDING',
    executedQuantity: 0,
    timestamp: new Date()
  };
  
  recentOrders.value.unshift(newOrder);
  
  // Simulate order execution after a delay
  setTimeout(() => {
    newOrder.status = 'EXECUTED';
    newOrder.executedQuantity = order.quantity;
    newOrder.executedPrice = order.price || currentSpotPrice.value;
  }, 2000);
};

const calculateOrderValue = (order: OrderRequest): string => {
  const price = order.price || currentSpotPrice.value;
  const value = price * order.quantity * 50; // Assuming 50 lot size
  return value.toLocaleString();
};

const getOrderStatusColor = (status: string): string => {
  switch (status) {
    case 'EXECUTED': return 'positive';
    case 'PENDING': return 'warning';
    case 'REJECTED': return 'negative';
    case 'CANCELLED': return 'grey';
    default: return 'info';
  }
};

const refreshData = async () => {
  isLoading.value = true;
  try {
    await Promise.all([
      loadOptionsChain(),
      loadPriceComparisons()
    ]);
  } finally {
    isLoading.value = false;
  }
};

const refreshPricing = async () => {
  await loadPriceComparisons();
};

const loadOptionsChain = async () => {
  // Simulate API call to load options chain
  await new Promise(resolve => setTimeout(resolve, 500));
  
  // Mock options chain data
  optionsChainData.value = {
    underlying: selectedUnderlying.value,
    expiry: new Date('2024-01-25'),
    spotPrice: currentSpotPrice.value,
    calls: [],
    puts: []
  };
};

const loadPriceComparisons = async () => {
  // Simulate API call to load price comparisons
  await new Promise(resolve => setTimeout(resolve, 300));
  
  // Mock price comparison data
  priceComparisons.value = [
    {
      symbol: 'NIFTY50',
      instrumentType: 'CASH',
      nsePrice: 19850.50,
      bsePrice: 19851.25,
      fairValue: 19850.75,
      percentageDiff: -0.01,
      isRecommended: true,
      liquidityScore: 0.95,
      volume30Day: 500000,
      currentVolume: 450000,
      openInterest: 0,
      exchange: 'NSE',
      bidAskSpread: 0.05,
      impactCost: 0.02,
      volumeCompliant: true
    }
  ];
};

// Lifecycle
onMounted(() => {
  refreshData();
});
</script>

<style scoped>
.trading-dashboard {
  height: 100vh;
  overflow: hidden;
}

.dashboard-header {
  height: 60px;
}

.left-panel, .right-panel {
  height: 100%;
  overflow-y: auto;
}

.strategy-leg {
  border-left: 3px solid #1976d2;
  border-radius: 4px;
}

.order-item {
  border-bottom: 1px solid #e0e0e0;
}

.order-details {
  background-color: #f5f5f5;
  padding: 16px;
  border-radius: 8px;
}

.market-status {
  text-align: center;
}

.portfolio-summary {
  text-align: right;
  min-width: 120px;
}
</style>