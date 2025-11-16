import React, { useEffect, useState } from 'react';
import { useDispatch, useSelector } from 'react-redux';
import {
  Box,
  Typography,
  AppBar,
  Toolbar,
  Container,
  Alert,
  Snackbar,
  CircularProgress,
  Paper,
} from '@mui/material';
import { RootState, AppDispatch } from './store/store';
import { fetchMarketData, setWebSocketConnected, updateMarketDataFromWS } from './store/marketDataSlice';
import { fetchConfig } from './store/configSlice';
import { fetchBaskets } from './store/basketsSlice';
import TradingGrid from './components/TradingGrid';
import ConfigurationPanel from './components/ConfigurationPanel';
import WebSocketManager from './utils/WebSocketManager';

function App() {
  const dispatch = useDispatch<AppDispatch>();
  const { data, loading, error, wsConnected } = useSelector((state: RootState) => state.marketData);
  const { error: configError } = useSelector((state: RootState) => state.config);
  const { error: basketsError } = useSelector((state: RootState) => state.baskets);
  
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');

  useEffect(() => {
    // Initialize data
    dispatch(fetchMarketData());
    dispatch(fetchConfig());
    dispatch(fetchBaskets());

    // Initialize WebSocket connection
    const wsManager = WebSocketManager.getInstance();
    
    wsManager.onConnect(() => {
      dispatch(setWebSocketConnected(true));
      console.log('WebSocket connected');
    });

    wsManager.onDisconnect(() => {
      dispatch(setWebSocketConnected(false));
      console.log('WebSocket disconnected');
    });

    wsManager.onMessage((data) => {
      if (data.type === 'MARKET_UPDATE' || data.type === 'INITIAL_DATA') {
        dispatch(updateMarketDataFromWS(data.data));
      }
    });

    wsManager.connect();

    return () => {
      wsManager.disconnect();
    };
  }, [dispatch]);

  useEffect(() => {
    if (error || configError || basketsError) {
      setSnackbarMessage(error || configError || basketsError || 'An error occurred');
      setSnackbarOpen(true);
    }
  }, [error, configError, basketsError]);

  const handleCloseSnackbar = () => {
    setSnackbarOpen(false);
  };

  if (loading && data.length === 0) {
    return (
      <Box
        display="flex"
        justifyContent="center"
        alignItems="center"
        minHeight="100vh"
        flexDirection="column"
        gap={2}
      >
        <CircularProgress size={60} />
        <Typography variant="h6" color="text.secondary">
          Loading Cash Futures THV Application...
        </Typography>
      </Box>
    );
  }

  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', height: '100vh' }}>
      {/* App Bar */}
      <AppBar position="static" elevation={1} sx={{ bgcolor: 'white', color: 'black' }}>
        <Toolbar>
          <Typography variant="h6" component="div" sx={{ flexGrow: 1, fontWeight: 600 }}>
            Cash Futures THV Trading Application
          </Typography>
          <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
            <Typography variant="body2" color="text.secondary">
              Trade Date: {new Date().toISOString().split('T')[0]}
            </Typography>
            <Box
              sx={{
                width: 12,
                height: 12,
                borderRadius: '50%',
                bgcolor: wsConnected ? 'success.main' : 'error.main',
                animation: wsConnected ? 'none' : 'pulse 2s infinite',
              }}
            />
            <Typography variant="body2" color="text.secondary">
              {wsConnected ? 'Connected' : 'Disconnected'}
            </Typography>
          </Box>
        </Toolbar>
      </AppBar>

      {/* Main Content */}
      <Box sx={{ display: 'flex', flex: 1, overflow: 'hidden' }}>
        {/* Configuration Panel */}
        <Box sx={{ width: 300, borderRight: 1, borderColor: 'divider', bgcolor: 'background.paper' }}>
          <ConfigurationPanel />
        </Box>

        {/* Trading Grid */}
        <Box sx={{ flex: 1, overflow: 'hidden' }}>
          <TradingGrid data={data} />
        </Box>
      </Box>

      {/* Status Bar */}
      <Paper
        elevation={1}
        sx={{
          padding: 1,
          borderRadius: 0,
          borderTop: 1,
          borderColor: 'divider',
          bgcolor: 'grey.50',
        }}
      >
        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          <Typography variant="caption" color="text.secondary">
            {data.length} instruments loaded | Last update: {new Date().toLocaleTimeString()}
          </Typography>
          <Typography variant="caption" color="text.secondary">
            Model: Statistical | Type: Equity | Data Set: 7 5 2
          </Typography>
        </Box>
      </Paper>

      {/* Error Snackbar */}
      <Snackbar
        open={snackbarOpen}
        autoHideDuration={6000}
        onClose={handleCloseSnackbar}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
      >
        <Alert onClose={handleCloseSnackbar} severity="error" sx={{ width: '100%' }}>
          {snackbarMessage}
        </Alert>
      </Snackbar>

      <style jsx>{`
        @keyframes pulse {
          0% {
            opacity: 1;
          }
          50% {
            opacity: 0.5;
          }
          100% {
            opacity: 1;
          }
        }
      `}</style>
    </Box>
  );
}

export default App;