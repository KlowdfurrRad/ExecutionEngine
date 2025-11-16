import { createSlice, createAsyncThunk, PayloadAction } from '@reduxjs/toolkit';

export interface Basket {
  name: string;
  description: string;
  stocks: string[];
  weightages: number[];
  created: number;
}

interface BasketsState {
  baskets: Basket[];
  loading: boolean;
  error: string | null;
}

const initialState: BasketsState = {
  baskets: [],
  loading: false,
  error: null,
};

export const fetchBaskets = createAsyncThunk(
  'baskets/fetchBaskets',
  async () => {
    const response = await fetch('/api/baskets');
    if (!response.ok) {
      throw new Error('Failed to fetch baskets');
    }
    return await response.json();
  }
);

export const createBasket = createAsyncThunk(
  'baskets/createBasket',
  async (basketData: Omit<Basket, 'created'>) => {
    const response = await fetch('/api/baskets', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(basketData),
    });
    if (!response.ok) {
      throw new Error('Failed to create basket');
    }
    return await response.json();
  }
);

export const deleteBasket = createAsyncThunk(
  'baskets/deleteBasket',
  async (name: string) => {
    const response = await fetch(`/api/baskets/${name}`, {
      method: 'DELETE',
    });
    if (!response.ok) {
      throw new Error('Failed to delete basket');
    }
    return name;
  }
);

const basketsSlice = createSlice({
  name: 'baskets',
  initialState,
  reducers: {
    clearError: (state) => {
      state.error = null;
    },
  },
  extraReducers: (builder) => {
    builder
      // Fetch baskets
      .addCase(fetchBaskets.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(fetchBaskets.fulfilled, (state, action) => {
        state.loading = false;
        state.baskets = action.payload;
      })
      .addCase(fetchBaskets.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to fetch baskets';
      })
      // Create basket
      .addCase(createBasket.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(createBasket.fulfilled, (state, action) => {
        state.loading = false;
        state.baskets.push(action.payload);
      })
      .addCase(createBasket.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to create basket';
      })
      // Delete basket
      .addCase(deleteBasket.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(deleteBasket.fulfilled, (state, action) => {
        state.loading = false;
        state.baskets = state.baskets.filter(basket => basket.name !== action.payload);
      })
      .addCase(deleteBasket.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to delete basket';
      });
  },
});

export const { clearError } = basketsSlice.actions;
export default basketsSlice.reducer;