import { createSlice, createAsyncThunk, PayloadAction } from '@reduxjs/toolkit';

export interface MarketDataItem {
  ticker: string;
  spot: number;
  volume: number;
  exchanges: string[];
  futures: {
    price: number;
    volume: number;
    oi: number;
    expiry: string;
    bid: number;
    ask: number;
  };
  options: {
    calls: {
      bid: number;
      ask: number;
      ltp: number;
      volume: number;
      oi: number;
    };
    puts: {
      bid: number;
      ask: number;
      ltp: number;
      volume: number;
      oi: number;
    };
  };
  calculations?: {
    theoretical_value: number;
    one_sdv: number;
    two_sdv: number;
    three_sdv: number;
    call_price: number;
    put_price: number;
    mean_percent: number;
    act_difference: number;
    percentage_over_cash: number;
    futures_cash_diff: number;
  };
  dividends: {
    announced: boolean;
    exDate: string | null;
    amount: number;
  };
}

interface MarketDataState {
  data: MarketDataItem[];
  loading: boolean;
  error: string | null;
  lastUpdate: number | null;
  wsConnected: boolean;
}

const initialState: MarketDataState = {
  data: [],
  loading: false,
  error: null,
  lastUpdate: null,
  wsConnected: false,
};

// Async thunks
export const fetchMarketData = createAsyncThunk(
  'marketData/fetchMarketData',
  async () => {
    const response = await fetch('/api/market-data');
    if (!response.ok) {
      throw new Error('Failed to fetch market data');
    }
    return await response.json();
  }
);

export const updateTicker = createAsyncThunk(
  'marketData/updateTicker',
  async ({ ticker, data }: { ticker: string; data: Partial<MarketDataItem> }) => {
    const response = await fetch(`/api/market-data/${ticker}`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(data),
    });
    if (!response.ok) {
      throw new Error('Failed to update ticker');
    }
    return await response.json();
  }
);

const marketDataSlice = createSlice({
  name: 'marketData',
  initialState,
  reducers: {
    setWebSocketConnected: (state, action: PayloadAction<boolean>) => {
      state.wsConnected = action.payload;
    },
    updateMarketDataFromWS: (state, action: PayloadAction<MarketDataItem[]>) => {
      state.data = action.payload;
      state.lastUpdate = Date.now();
    },
    clearError: (state) => {
      state.error = null;
    },
    updateSingleTicker: (state, action: PayloadAction<MarketDataItem>) => {
      const index = state.data.findIndex(item => item.ticker === action.payload.ticker);
      if (index !== -1) {
        state.data[index] = action.payload;
      } else {
        state.data.push(action.payload);
      }
      state.lastUpdate = Date.now();
    },
  },
  extraReducers: (builder) => {
    builder
      // Fetch market data
      .addCase(fetchMarketData.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(fetchMarketData.fulfilled, (state, action) => {
        state.loading = false;
        state.data = action.payload;
        state.lastUpdate = Date.now();
      })
      .addCase(fetchMarketData.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to fetch market data';
      })
      // Update ticker
      .addCase(updateTicker.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(updateTicker.fulfilled, (state, action) => {
        state.loading = false;
        const index = state.data.findIndex(item => item.ticker === action.payload.ticker);
        if (index !== -1) {
          state.data[index] = action.payload;
        } else {
          state.data.push(action.payload);
        }
        state.lastUpdate = Date.now();
      })
      .addCase(updateTicker.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to update ticker';
      });
  },
});

export const { 
  setWebSocketConnected, 
  updateMarketDataFromWS, 
  clearError, 
  updateSingleTicker 
} = marketDataSlice.actions;

export default marketDataSlice.reducer;