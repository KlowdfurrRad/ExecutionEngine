import React, { useState } from 'react';
import { useSelector, useDispatch } from 'react-redux';
import {
  Box,
  Paper,
  Typography,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Switch,
  FormControlLabel,
  Divider,
  Button,
  Accordion,
  AccordionSummary,
  AccordionDetails,
  Chip,
  Grid,
  Tooltip,
  IconButton,
} from '@mui/material';
import {
  ExpandMore,
  Add,
  Delete,
  Settings,
  Refresh,
  Save,
} from '@mui/icons-material';
import { RootState, AppDispatch } from '../store/store';
import { updateInterestRate, toggleVix } from '../store/configSlice';

const ConfigurationPanel: React.FC = () => {
  const dispatch = useDispatch<AppDispatch>();
  const { interestRates, exchanges, vixEnabled } = useSelector((state: RootState) => state.config);
  const [tradeDate, setTradeDate] = useState(new Date().toISOString().split('T')[0]);
  const [modelType, setModelType] = useState('Statistical');
  const [dataSet, setDataSet] = useState('7 5 2');
  const [basketWeight, setBasketWeight] = useState('80 10 10');

  const handleInterestRateChange = (period: string, value: string) => {
    const rate = parseFloat(value);
    if (!isNaN(rate)) {
      dispatch(updateInterestRate({ period, rate }));
    }
  };

  return (
    <Box sx={{ height: '100%', overflow: 'auto', p: 2 }}>
      <Typography variant="h6" sx={{ mb: 2, fontWeight: 600 }}>
        Configuration
      </Typography>

      {/* Trading Configuration */}
      <Accordion defaultExpanded>
        <AccordionSummary expandIcon={<ExpandMore />}>
          <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
            Trading Setup
          </Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
            <TextField
              label="Trade Date"
              type="date"
              value={tradeDate}
              onChange={(e) => setTradeDate(e.target.value)}
              size="small"
              fullWidth
              InputLabelProps={{ shrink: true }}
            />
            
            <FormControl size="small" fullWidth>
              <InputLabel>Model</InputLabel>
              <Select
                value={modelType}
                label="Model"
                onChange={(e) => setModelType(e.target.value)}
              >
                <MenuItem value="Statistical">Statistical</MenuItem>
                <MenuItem value="Black-Scholes">Black-Scholes</MenuItem>
                <MenuItem value="Monte Carlo">Monte Carlo</MenuItem>
              </Select>
            </FormControl>

            <TextField
              label="Type"
              value="Equity"
              size="small"
              fullWidth
              disabled
            />

            <TextField
              label="Data Set"
              value={dataSet}
              onChange={(e) => setDataSet(e.target.value)}
              size="small"
              fullWidth
              helperText="Format: days interval"
            />
          </Box>
        </AccordionDetails>
      </Accordion>

      {/* Interest Rates */}
      <Accordion>
        <AccordionSummary expandIcon={<ExpandMore />}>
          <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
            Interest Rates (%)
          </Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Grid container spacing={1}>
            {Object.entries(interestRates).map(([period, rate]) => (
              <Grid item xs={6} key={period}>
                <TextField
                  label={`${period} days`}
                  type="number"
                  value={rate}
                  onChange={(e) => handleInterestRateChange(period, e.target.value)}
                  size="small"
                  fullWidth
                  inputProps={{ step: 0.1, min: 0, max: 20 }}
                />
              </Grid>
            ))}
          </Grid>
        </AccordionDetails>
      </Accordion>

      {/* Exchanges */}
      <Accordion>
        <AccordionSummary expandIcon={<ExpandMore />}>
          <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
            Exchanges
          </Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1 }}>
            {exchanges.map((exchange) => (
              <Chip
                key={exchange}
                label={exchange}
                variant="outlined"
                size="small"
              />
            ))}
          </Box>
        </AccordionDetails>
      </Accordion>

      {/* Expiries */}
      <Accordion>
        <AccordionSummary expandIcon={<ExpandMore />}>
          <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
            Expiries (Days)
          </Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Typography variant="body2" color="text.secondary">
            7, 30, 60, 90, 180, 365
          </Typography>
        </AccordionDetails>
      </Accordion>

      {/* Baskets */}
      <Accordion>
        <AccordionSummary expandIcon={<ExpandMore />}>
          <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
            Baskets & Weightages
          </Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
            <TextField
              label="Basket Weights"
              value={basketWeight}
              onChange={(e) => setBasketWeight(e.target.value)}
              size="small"
              fullWidth
              helperText="Format: weight1 weight2 weight3"
            />
            
            <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
              <Typography variant="body2" color="text.secondary">
                Available Baskets:
              </Typography>
              {['Basket 1', 'Basket 2', 'Basket 3', 'Basket 4', 'Basket 5', 'Basket 6'].map((basket, index) => (
                <Box key={basket} sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                  <Typography variant="body2">{basket}</Typography>
                  <Box>
                    <Tooltip title="Edit Basket">
                      <IconButton size="small">
                        <Settings fontSize="small" />
                      </IconButton>
                    </Tooltip>
                    <Tooltip title="Delete Basket">
                      <IconButton size="small">
                        <Delete fontSize="small" />
                      </IconButton>
                    </Tooltip>
                  </Box>
                </Box>
              ))}
              
              <Button
                startIcon={<Add />}
                variant="outlined"
                size="small"
                fullWidth
                sx={{ mt: 1 }}
              >
                Add New Basket
              </Button>
            </Box>
          </Box>
        </AccordionDetails>
      </Accordion>

      {/* Dividends */}
      <Accordion>
        <AccordionSummary expandIcon={<ExpandMore />}>
          <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
            Dividends
          </Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
            <Typography variant="body2" color="text.secondary">
              Dividend slots: Divs 1-18
            </Typography>
            <Button
              startIcon={<Add />}
              variant="outlined"
              size="small"
              fullWidth
            >
              Add Dividend Event
            </Button>
          </Box>
        </AccordionDetails>
      </Accordion>

      {/* VIX and Options */}
      <Accordion>
        <AccordionSummary expandIcon={<ExpandMore />}>
          <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
            Advanced Settings
          </Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
            <FormControlLabel
              control={
                <Switch
                  checked={vixEnabled}
                  onChange={() => dispatch(toggleVix())}
                  size="small"
                />
              }
              label="VIX Enabled"
            />
            
            <TextField
              label="SDV Levels"
              value="1, 2, 3"
              size="small"
              fullWidth
              disabled
            />
          </Box>
        </AccordionDetails>
      </Accordion>

      <Divider sx={{ my: 2 }} />

      {/* Action Buttons */}
      <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
        <Button
          startIcon={<Save />}
          variant="contained"
          size="small"
          fullWidth
        >
          Save Configuration
        </Button>
        
        <Button
          startIcon={<Refresh />}
          variant="outlined"
          size="small"
          fullWidth
        >
          Reset to Default
        </Button>
      </Box>

      {/* Summary Info */}
      <Paper 
        elevation={1} 
        sx={{ 
          mt: 2, 
          p: 1, 
          bgcolor: 'grey.50',
          border: '1px solid',
          borderColor: 'divider'
        }}
      >
        <Typography variant="caption" display="block" gutterBottom>
          <strong>Mean:</strong> Statistical
        </Typography>
        <Typography variant="caption" display="block" gutterBottom>
          <strong>SDV1:</strong> 1 Standard Deviation
        </Typography>
        <Typography variant="caption" display="block" gutterBottom>
          <strong>SDV2:</strong> 2 Standard Deviations
        </Typography>
        <Typography variant="caption" display="block">
          <strong>SDV3:</strong> 3 Standard Deviations
        </Typography>
      </Paper>
    </Box>
  );
};

export default ConfigurationPanel;