import { configureStore } from '@reduxjs/toolkit';
import marketDataReducer from './marketDataSlice';
import configReducer from './configSlice';
import basketsReducer from './basketsSlice';

export const store = configureStore({
  reducer: {
    marketData: marketDataReducer,
    config: configReducer,
    baskets: basketsReducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      serializableCheck: {
        ignoredActions: ['persist/PERSIST'],
      },
    }),
});

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;