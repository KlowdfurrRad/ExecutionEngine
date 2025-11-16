const express = require('express');
const cors = require('cors');
const WebSocket = require('ws');
const http = require('http');
const cron = require('node-cron');
const { v4: uuidv4 } = require('uuid');
require('dotenv').config();

const app = express();
const server = http.createServer(app);

// Middleware
app.use(cors({
    origin: ['http://localhost:3000', 'http://localhost:3001'],
    credentials: true
}));
app.use(express.json());

// Financial calculations
class FinancialCalculator {
    static normalCDF(x) {
        const a1 = 0.254829592;
        const a2 = -0.284496736;
        const a3 = 1.421413741;
        const a4 = -1.453152027;
        const a5 = 1.061405429;
        const p = 0.3275911;
        
        const sign = x >= 0 ? 1 : -1;
        x = Math.abs(x);
        
        const t = 1.0 / (1.0 + p * x);
        const y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * Math.exp(-x * x);
        
        return 0.5 * (1.0 + sign * y);
    }

    static blackScholesCall(S, K, r, T, sigma) {
        if (T <= 0) return Math.max(S - K, 0);
        
        const d1 = (Math.log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * Math.sqrt(T));
        const d2 = d1 - sigma * Math.sqrt(T);
        
        return S * this.normalCDF(d1) - K * Math.exp(-r * T) * this.normalCDF(d2);
    }
    
    static blackScholesPut(S, K, r, T, sigma) {
        if (T <= 0) return Math.max(K - S, 0);
        
        const d1 = (Math.log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * Math.sqrt(T));
        const d2 = d1 - sigma * Math.sqrt(T);
        
        return K * Math.exp(-r * T) * this.normalCDF(-d2) - S * this.normalCDF(-d1);
    }

    static calculateTheoreticalValue(spot, rate, time) {
        return spot * Math.exp(rate * time);
    }

    static calculateSDV(spot, volatility, time) {
        return spot * volatility * Math.sqrt(time);
    }
}

// Global data storage
let marketData = new Map();
let baskets = new Map();
const wsConnections = new Set();

// Initialize sample market data
function initializeMarketData() {
    const tickers = ['HDFCBANK', 'AXISBANK', 'RELIANCE', 'TCS', 'INFY', 'ICICIBANK', 'SBIN', 'WIPRO', 'LT', 'BAJFINANCE'];
    
    tickers.forEach(ticker => {
        const spot = Math.round((400 + Math.random() * 200) * 100) / 100;
        const volume = Math.floor(10000 + Math.random() * 90000);
        
        marketData.set(ticker, {
            ticker,
            spot,
            volume,
            exchanges: ['NSE', 'BSE'],
            futures: {
                price: Math.round((spot * 1.01) * 100) / 100,
                volume: Math.floor(volume / 2),
                oi: volume * 3,
                expiry: '28NOV25',
                bid: Math.round((spot * 0.995) * 100) / 100,
                ask: Math.round((spot * 1.005) * 100) / 100
            },
            options: {
                calls: {
                    bid: Math.round((15 + Math.random() * 30) * 100) / 100,
                    ask: Math.round((15 + Math.random() * 30) * 100) / 100,
                    ltp: Math.round((15 + Math.random() * 30) * 100) / 100,
                    volume: Math.floor(volume / 10),
                    oi: Math.floor(volume / 2)
                },
                puts: {
                    bid: Math.round((15 + Math.random() * 30) * 100) / 100,
                    ask: Math.round((15 + Math.random() * 30) * 100) / 100,
                    ltp: Math.round((15 + Math.random() * 30) * 100) / 100,
                    volume: Math.floor(volume / 10),
                    oi: Math.floor(volume / 2)
                }
            },
            dividends: {
                announced: false,
                exDate: null,
                amount: 0
            }
        });
    });
}

// Enhanced market data with calculations
function getEnrichedMarketData() {
    const enrichedData = [];
    
    for (const [ticker, data] of marketData) {
        const spot = data.spot;
        const rate = 0.064; // 6.4% default
        const time = 30 / 365; // 30 days
        const vol = 0.25; // 25% volatility
        
        const theoreticalValue = FinancialCalculator.calculateTheoreticalValue(spot, rate, time);
        const sdv = FinancialCalculator.calculateSDV(spot, vol, time);
        const callPrice = FinancialCalculator.blackScholesCall(spot, spot, rate, time, vol);
        const putPrice = FinancialCalculator.blackScholesPut(spot, spot, rate, time, vol);
        
        const enriched = {
            ...data,
            calculations: {
                theoretical_value: Math.round(theoreticalValue * 100) / 100,
                one_sdv: Math.round(sdv * 100) / 100,
                two_sdv: Math.round(2 * sdv * 100) / 100,
                three_sdv: Math.round(3 * sdv * 100) / 100,
                call_price: Math.round(callPrice * 100) / 100,
                put_price: Math.round(putPrice * 100) / 100,
                mean_percent: Math.round((25 + Math.random() * 5) * 10) / 10,
                act_difference: Math.round((Math.random() - 0.5) * 2 * 1000) / 1000,
                percentage_over_cash: Math.round(((data.futures.price - spot) / spot * 100) * 1000) / 1000,
                futures_cash_diff: Math.round((data.futures.price - spot) * 100) / 100
            }
        };
        
        enrichedData.push(enriched);
    }
    
    return enrichedData;
}

// WebSocket setup
const wss = new WebSocket.Server({ server });

wss.on('connection', (ws) => {
    wsConnections.add(ws);
    console.log('WebSocket client connected. Total clients:', wsConnections.size);
    
    // Send initial data
    const initialData = {
        type: 'INITIAL_DATA',
        data: getEnrichedMarketData(),
        timestamp: Date.now()
    };
    ws.send(JSON.stringify(initialData));
    
    ws.on('close', () => {
        wsConnections.delete(ws);
        console.log('WebSocket client disconnected. Total clients:', wsConnections.size);
    });
    
    ws.on('message', (data) => {
        console.log('Received WebSocket message:', data.toString());
    });
});

// Broadcast market updates
function broadcastMarketUpdate() {
    // Update prices with random walk
    for (const [ticker, data] of marketData) {
        const change = (Math.random() - 0.5) * 2; // -1 to +1
        const newSpot = Math.max(1, data.spot + change);
        data.spot = Math.round(newSpot * 100) / 100;
        
        // Update futures price
        data.futures.price = Math.round((newSpot * 1.01) * 100) / 100;
        data.futures.bid = Math.round((newSpot * 0.995) * 100) / 100;
        data.futures.ask = Math.round((newSpot * 1.005) * 100) / 100;
    }
    
    if (wsConnections.size > 0) {
        const update = {
            type: 'MARKET_UPDATE',
            data: getEnrichedMarketData(),
            timestamp: Date.now()
        };
        
        const message = JSON.stringify(update);
        wsConnections.forEach(ws => {
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(message);
            }
        });
    }
}

// API Routes

// Root endpoint
app.get('/', (req, res) => {
    res.json({
        message: 'Cash Futures THV API - Node.js Express Backend',
        version: '1.0.0',
        features: ['WebSocket', 'REST API', 'Real-time calculations']
    });
});

// Configuration endpoint
app.get('/api/config', (req, res) => {
    res.json({
        interest_rates: {
            7: 6.2,
            30: 6.4,
            60: 6.7,
            90: 6.9,
            180: 7.1
        },
        expiries: [7, 30, 60, 90, 180, 365],
        exchanges: ['NSE', 'BSE', 'MCX', 'NCDEX'],
        vix_enabled: true
    });
});

// Get all market data
app.get('/api/market-data', (req, res) => {
    res.json(getEnrichedMarketData());
});

// Get specific ticker
app.get('/api/market-data/:ticker', (req, res) => {
    const ticker = req.params.ticker.toUpperCase();
    
    if (marketData.has(ticker)) {
        res.json(marketData.get(ticker));
    } else {
        res.status(404).json({ error: 'Ticker not found' });
    }
});

// Update/Create ticker
app.post('/api/market-data/:ticker', (req, res) => {
    const ticker = req.params.ticker.toUpperCase();
    
    try {
        if (marketData.has(ticker)) {
            // Update existing
            const existing = marketData.get(ticker);
            marketData.set(ticker, { ...existing, ...req.body });
        } else {
            // Create new
            const newData = {
                ticker,
                spot: 0,
                volume: 0,
                exchanges: ['NSE'],
                futures: { price: 0, volume: 0, oi: 0, expiry: '', bid: 0, ask: 0 },
                options: {
                    calls: { bid: 0, ask: 0, ltp: 0, volume: 0, oi: 0 },
                    puts: { bid: 0, ask: 0, ltp: 0, volume: 0, oi: 0 }
                },
                dividends: { announced: false, exDate: null, amount: 0 },
                ...req.body
            };
            marketData.set(ticker, newData);
        }
        
        res.json(marketData.get(ticker));
    } catch (error) {
        res.status(400).json({ error: 'Invalid JSON' });
    }
});

// Baskets management
app.get('/api/baskets', (req, res) => {
    res.json(Array.from(baskets.entries()).map(([name, data]) => ({ name, ...data })));
});

app.post('/api/baskets', (req, res) => {
    try {
        const { name, description, stocks, weightages } = req.body;
        
        const basket = {
            description: description || '',
            stocks,
            weightages,
            created: Date.now()
        };
        
        baskets.set(name, basket);
        res.status(201).json({ name, ...basket });
    } catch (error) {
        res.status(400).json({ error: 'Invalid basket data' });
    }
});

app.delete('/api/baskets/:name', (req, res) => {
    const name = req.params.name;
    
    if (baskets.has(name)) {
        baskets.delete(name);
        res.json({ message: 'Basket deleted' });
    } else {
        res.status(404).json({ error: 'Basket not found' });
    }
});

// Initialize data and start server
initializeMarketData();

// Schedule market updates every 3 seconds
cron.schedule('*/3 * * * * *', broadcastMarketUpdate);

const PORT = process.env.PORT || 5001;

server.listen(PORT, () => {
    console.log('==================================================');
    console.log('🚀 Node.js Express Backend Starting...');
    console.log('==================================================');
    console.log(`Server: http://localhost:${PORT}`);
    console.log(`WebSocket: ws://localhost:${PORT}`);
    console.log(`Market Data: ${marketData.size} instruments loaded`);
    console.log('Features: Real-time calculations, WebSocket, REST API');
    console.log('==================================================');
});

module.exports = app;