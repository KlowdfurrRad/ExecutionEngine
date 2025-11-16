import React, { useCallback, useMemo, useState } from 'react';
import { AgGridReact } from 'ag-grid-react';
import { ColDef, GridReadyEvent, CellStyle, ValueFormatterParams } from 'ag-grid-community';
import { Box, Paper, Toolbar, Typography, Button, IconButton, Tooltip } from '@mui/material';
import { Download, Refresh, Add, Settings } from '@mui/icons-material';
import { MarketDataItem } from '../store/marketDataSlice';
import 'ag-grid-community/styles/ag-grid.css';
import 'ag-grid-community/styles/ag-theme-alpine.css';

interface TradingGridProps {
  data: MarketDataItem[];
}

const TradingGrid: React.FC<TradingGridProps> = ({ data }) => {
  const [gridApi, setGridApi] = useState<any>(null);

  // Number formatter for financial data
  const numberFormatter = (params: ValueFormatterParams): string => {
    if (params.value == null) return '';
    return typeof params.value === 'number' ? params.value.toFixed(2) : params.value.toString();
  };

  const percentageFormatter = (params: ValueFormatterParams): string => {
    if (params.value == null) return '';
    return typeof params.value === 'number' ? `${params.value.toFixed(3)}%` : params.value.toString();
  };

  // Cell style functions for positive/negative values
  const getValueCellStyle = useCallback((params: any): CellStyle => {
    if (params.value > 0) {
      return { color: '#198754', backgroundColor: 'rgba(25, 135, 84, 0.1)' };
    } else if (params.value < 0) {
      return { color: '#dc3545', backgroundColor: 'rgba(220, 53, 69, 0.1)' };
    }
    return {};
  }, []);

  // Column definitions based on Excel layout
  const columnDefs: ColDef[] = useMemo(() => [
    // EQ Section
    {
      headerName: 'EQ',
      children: [
        {
          headerName: 'Ticker',
          field: 'ticker',
          width: 100,
          pinned: 'left',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Volume',
          field: 'volume',
          width: 90,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'NSE',
          field: 'spot',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Mean %',
          field: 'calculations.mean_percent',
          width: 80,
          valueFormatter: (params) => params.value ? `${params.value.toFixed(1)}` : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'BSE',
          field: 'spot',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Act Difference',
          field: 'calculations.act_difference',
          width: 110,
          valueFormatter: numberFormatter,
          cellStyle: getValueCellStyle,
          cellClass: 'excel-cell',
        },
      ]
    },
    // Futures Section
    {
      headerName: 'Fut',
      children: [
        {
          headerName: 'Volume',
          field: 'futures.volume',
          width: 90,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: '1SDV',
          field: 'calculations.one_sdv',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: '2 SDV',
          field: 'calculations.two_sdv',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: '3 SDV',
          field: 'calculations.three_sdv',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Expiry Series',
          field: 'futures.expiry',
          width: 110,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Bid',
          field: 'futures.bid',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Th Value',
          field: 'calculations.theoretical_value',
          width: 90,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Ask',
          field: 'futures.ask',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'LTP',
          field: 'futures.price',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: '% over Cash',
          field: 'calculations.percentage_over_cash',
          width: 100,
          valueFormatter: percentageFormatter,
          cellStyle: getValueCellStyle,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Future - Cash',
          field: 'calculations.futures_cash_diff',
          width: 110,
          valueFormatter: numberFormatter,
          cellStyle: getValueCellStyle,
          cellClass: 'excel-cell',
        },
      ]
    },
    // Options Section
    {
      headerName: 'Option',
      children: [
        {
          headerName: 'Call - Put',
          field: 'calculations.call_price',
          width: 90,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: '1SDV',
          field: 'calculations.one_sdv',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: '2 SDV',
          field: 'calculations.two_sdv',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: '3 SDV',
          field: 'calculations.three_sdv',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Future Vol',
          field: 'futures.volume',
          width: 90,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Future OI',
          field: 'futures.oi',
          width: 90,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Expiry',
          field: 'futures.expiry',
          width: 80,
          cellClass: 'excel-cell',
        },
      ]
    },
    // Call Options
    {
      headerName: 'Call',
      children: [
        {
          headerName: 'Call OI',
          field: 'options.calls.oi',
          width: 80,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Call Vol',
          field: 'options.calls.volume',
          width: 80,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Bid',
          field: 'options.calls.bid',
          width: 70,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Ask',
          field: 'options.calls.ask',
          width: 70,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'LTP',
          field: 'options.calls.ltp',
          width: 70,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
      ]
    },
    // Put Options
    {
      headerName: 'Put',
      children: [
        {
          headerName: 'Put OI',
          field: 'options.puts.oi',
          width: 80,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Put Vol',
          field: 'options.puts.volume',
          width: 80,
          valueFormatter: (params) => params.value ? params.value.toLocaleString() : '',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Bid',
          field: 'options.puts.bid',
          width: 70,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Ask',
          field: 'options.puts.ask',
          width: 70,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
        {
          headerName: 'LTP',
          field: 'options.puts.ltp',
          width: 70,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
      ]
    },
    // Dividends Section
    {
      headerName: 'Divs',
      children: [
        {
          headerName: 'Div Announced',
          field: 'dividends.announced',
          width: 110,
          valueFormatter: (params) => params.value ? 'Yes' : 'No',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'Ex Date/Div Date',
          field: 'dividends.exDate',
          width: 130,
          valueFormatter: (params) => params.value || '-',
          cellClass: 'excel-cell',
        },
        {
          headerName: 'BDVD',
          field: 'dividends.amount',
          width: 80,
          valueFormatter: numberFormatter,
          cellClass: 'excel-cell',
        },
      ]
    },
  ], [getValueCellStyle]);

  const defaultColDef = useMemo(() => ({
    sortable: true,
    filter: true,
    resizable: true,
    minWidth: 70,
  }), []);

  const onGridReady = useCallback((params: GridReadyEvent) => {
    setGridApi(params.api);
  }, []);

  const handleExport = useCallback(() => {
    if (gridApi) {
      gridApi.exportDataAsCsv({
        fileName: `cash_futures_thv_${new Date().toISOString().split('T')[0]}.csv`,
      });
    }
  }, [gridApi]);

  const handleRefresh = useCallback(() => {
    // Refresh will be handled by parent component through WebSocket
    console.log('Refresh requested');
  }, []);

  const handleAddStock = useCallback(() => {
    // TODO: Implement add stock functionality
    console.log('Add stock requested');
  }, []);

  return (
    <Box sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
      {/* Toolbar */}
      <Paper elevation={1} sx={{ borderRadius: 0 }}>
        <Toolbar variant="dense" sx={{ minHeight: 48 }}>
          <Typography variant="subtitle1" sx={{ flexGrow: 1, fontWeight: 600 }}>
            Market Data Grid
          </Typography>
          <Box sx={{ display: 'flex', gap: 1 }}>
            <Tooltip title="Add Stock/Basket">
              <IconButton size="small" onClick={handleAddStock}>
                <Add />
              </IconButton>
            </Tooltip>
            <Tooltip title="Refresh Data">
              <IconButton size="small" onClick={handleRefresh}>
                <Refresh />
              </IconButton>
            </Tooltip>
            <Tooltip title="Export to CSV">
              <IconButton size="small" onClick={handleExport}>
                <Download />
              </IconButton>
            </Tooltip>
            <Tooltip title="Grid Settings">
              <IconButton size="small">
                <Settings />
              </IconButton>
            </Tooltip>
          </Box>
        </Toolbar>
      </Paper>

      {/* Data Grid */}
      <Box sx={{ flex: 1, position: 'relative' }}>
        <div className="ag-theme-alpine" style={{ height: '100%', width: '100%' }}>
          <AgGridReact
            rowData={data}
            columnDefs={columnDefs}
            defaultColDef={defaultColDef}
            onGridReady={onGridReady}
            rowHeight={24}
            headerHeight={32}
            animateRows={true}
            rowSelection="multiple"
            suppressRowClickSelection={false}
            enableRangeSelection={true}
            enableCellTextSelection={true}
            suppressMenuHide={true}
            getRowId={(params) => params.data.ticker}
          />
        </div>
      </Box>
    </Box>
  );
};

export default TradingGrid;