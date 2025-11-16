import { createSlice, createAsyncThunk, PayloadAction } from '@reduxjs/toolkit';

export interface ConfigState {
  interestRates: Record<string, number>;
  expiries: number[];
  exchanges: string[];
  vixEnabled: boolean;
  loading: boolean;
  error: string | null;
}

const initialState: ConfigState = {
  interestRates: {
    '7': 6.2,
    '30': 6.4,
    '60': 6.7,
    '90': 6.9,
    '180': 7.1
  },
  expiries: [7, 30, 60, 90, 180, 365],
  exchanges: ['NSE', 'BSE', 'MCX', 'NCDEX'],
  vixEnabled: true,
  loading: false,
  error: null,
};

export const fetchConfig = createAsyncThunk(
  'config/fetchConfig',
  async () => {
    const response = await fetch('/api/config');
    if (!response.ok) {
      throw new Error('Failed to fetch configuration');
    }
    return await response.json();
  }
);

const configSlice = createSlice({
  name: 'config',
  initialState,
  reducers: {
    updateInterestRate: (state, action: PayloadAction<{ period: string; rate: number }>) => {
      state.interestRates[action.payload.period] = action.payload.rate;
    },
    toggleVix: (state) => {
      state.vixEnabled = !state.vixEnabled;
    },
    clearError: (state) => {
      state.error = null;
    },
  },
  extraReducers: (builder) => {
    builder
      .addCase(fetchConfig.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(fetchConfig.fulfilled, (state, action) => {
        state.loading = false;
        state.interestRates = action.payload.interest_rates;
        state.expiries = action.payload.expiries;
        state.exchanges = action.payload.exchanges;
        state.vixEnabled = action.payload.vix_enabled;
      })
      .addCase(fetchConfig.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to fetch configuration';
      });
  },
});

export const { updateInterestRate, toggleVix, clearError } = configSlice.actions;
export default configSlice.reducer;