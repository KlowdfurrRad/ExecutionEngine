from fastapi import FastAPI, WebSocket, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
import asyncio
import numpy as np
import pandas as pd
from scipy.stats import norm
from datetime import datetime, timedelta
import json
import uvicorn
from typing import List, Dict, Optional
from pydantic import BaseModel
import logging
import websockets
from concurrent.futures import ThreadPoolExecutor
import yfinance as yf
import requests

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="Cash Futures THV API", version="2.0.0")

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Data models
class StockData(BaseModel):
    ticker: str
    spot: float
    volume: int
    exchanges: List[str]
    futures: Dict
    options: Dict
    dividends: Dict

class BasketData(BaseModel):
    name: str
    description: Optional[str] = ""
    stocks: List[str]
    weightages: List[float]

class MarketConfig(BaseModel):
    interest_rates: Dict[str, float]
    expiries: List[int]
    exchanges: List[str]
    vix_enabled: bool

# Global data storage (in production, use Redis/database)
market_data = {}
baskets = {}
active_connections = []

# Financial calculation utilities using numpy for vectorized operations
class OptimizedFinancialCalculations:
    
    @staticmethod
    def black_scholes_vectorized(S, K, r, T, sigma, option_type='call'):
        """Vectorized Black-Scholes calculation using numpy"""
        S = np.array(S, dtype=float)
        K = np.array(K, dtype=float)
        r = np.array(r, dtype=float)
        T = np.array(T, dtype=float)
        sigma = np.array(sigma, dtype=float)
        
        d1 = (np.log(S / K) + (r + 0.5 * sigma**2) * T) / (sigma * np.sqrt(T))
        d2 = d1 - sigma * np.sqrt(T)
        
        if option_type == 'call':
            return S * norm.cdf(d1) - K * np.exp(-r * T) * norm.cdf(d2)
        else:
            return K * np.exp(-r * T) * norm.cdf(-d2) - S * norm.cdf(-d1)
    
    @staticmethod
    def calculate_theoretical_values_batch(spots, rates, times, dividends=None, volatilities=None):
        """Calculate theoretical values for multiple stocks simultaneously"""
        spots = np.array(spots)
        rates = np.array(rates)
        times = np.array(times)
        
        if dividends is None:
            dividends = np.zeros_like(spots)
        else:
            dividends = np.array(dividends)
            
        if volatilities is None:
            volatilities = np.full_like(spots, 0.25)  # Default 25% volatility
        else:
            volatilities = np.array(volatilities)
        
        # Adjust spot prices for dividends
        adjusted_spots = spots - dividends
        
        # Calculate theoretical futures prices
        theoretical_values = adjusted_spots * np.exp(rates * times)
        
        # Calculate SDV levels
        sdv_base = spots * volatilities * np.sqrt(times)
        
        results = {
            'theoretical_values': theoretical_values.tolist(),
            'one_sdv': sdv_base.tolist(),
            'two_sdv': (2 * sdv_base).tolist(),
            'three_sdv': (3 * sdv_base).tolist()
        }
        
        return results
    
    @staticmethod
    def calculate_greeks_vectorized(S, K, r, T, sigma):
        """Calculate option Greeks using vectorized operations"""
        S = np.array(S)
        K = np.array(K)
        r = np.array(r)
        T = np.array(T)
        sigma = np.array(sigma)
        
        d1 = (np.log(S / K) + (r + 0.5 * sigma**2) * T) / (sigma * np.sqrt(T))
        d2 = d1 - sigma * np.sqrt(T)
        
        # Delta
        call_delta = norm.cdf(d1)
        put_delta = call_delta - 1
        
        # Gamma
        gamma = norm.pdf(d1) / (S * sigma * np.sqrt(T))
        
        # Theta
        call_theta = -(S * norm.pdf(d1) * sigma) / (2 * np.sqrt(T)) - r * K * np.exp(-r * T) * norm.cdf(d2)
        put_theta = -(S * norm.pdf(d1) * sigma) / (2 * np.sqrt(T)) + r * K * np.exp(-r * T) * norm.cdf(-d2)
        
        # Vega
        vega = S * np.sqrt(T) * norm.pdf(d1)
        
        return {
            'call_delta': call_delta.tolist(),
            'put_delta': put_delta.tolist(),
            'gamma': gamma.tolist(),
            'call_theta': call_theta.tolist(),
            'put_theta': put_theta.tolist(),
            'vega': vega.tolist()
        }

# Initialize with sample data
def initialize_sample_data():
    global market_data
    
    sample_tickers = ['HDFCBANK', 'AXISBANK', 'RELIANCE', 'TCS', 'INFY', 'ICICIBANK']
    
    for ticker in sample_tickers:
        market_data[ticker] = {
            "ticker": ticker,
            "spot": np.random.uniform(400, 500),
            "volume": np.random.randint(10000, 100000),
            "exchanges": ["NSE", "BSE"],
            "futures": {
                "price": 0,  # Will be calculated
                "volume": np.random.randint(10000, 50000),
                "oi": np.random.randint(50000, 200000),
                "expiry": "28NOV25",
                "bid": 0,
                "ask": 0
            },
            "options": {
                "calls": {
                    "bid": np.random.uniform(20, 40),
                    "ask": np.random.uniform(20, 40),
                    "ltp": np.random.uniform(20, 40),
                    "volume": np.random.randint(1000, 10000),
                    "oi": np.random.randint(10000, 50000)
                },
                "puts": {
                    "bid": np.random.uniform(20, 40),
                    "ask": np.random.uniform(20, 40),
                    "ltp": np.random.uniform(20, 40),
                    "volume": np.random.randint(1000, 10000),
                    "oi": np.random.randint(10000, 50000)
                }
            },
            "dividends": {
                "announced": False,
                "ex_date": None,
                "amount": 0
            }
        }

# Bloomberg API integration (mock for now)
async def fetch_bloomberg_data(ticker: str):
    """Fetch data from Bloomberg API or fallback to Yahoo Finance"""
    try:
        # In production, use actual Bloomberg API
        # For now, use yfinance as fallback
        stock = yf.Ticker(f"{ticker}.NS")  # NSE suffix for Indian stocks
        info = stock.history(period="1d")
        
        if not info.empty:
            latest = info.iloc[-1]
            return {
                "spot": float(latest['Close']),
                "volume": int(latest['Volume']),
                "timestamp": datetime.now().isoformat()
            }
    except Exception as e:
        logger.error(f"Error fetching data for {ticker}: {e}")
    
    # Return mock data if API fails
    return {
        "spot": np.random.uniform(400, 500),
        "volume": np.random.randint(10000, 100000),
        "timestamp": datetime.now().isoformat()
    }

# API Routes

@app.get("/")
async def root():
    return {"message": "Cash Futures THV API - Python FastAPI Backend"}

@app.get("/api/config")
async def get_config():
    return {
        "interest_rates": {
            "7": 6.2,
            "30": 6.4,
            "60": 6.7,
            "90": 6.9,
            "180": 7.1
        },
        "expiries": [7, 30, 60, 90, 180, 365],
        "exchanges": ["NSE", "BSE", "MCX", "NCDEX"],
        "vix_enabled": True
    }

@app.get("/api/market-data")
async def get_market_data():
    """Get all market data with optimized calculations"""
    if not market_data:
        initialize_sample_data()
    
    # Prepare data for vectorized calculations
    tickers = list(market_data.keys())
    spots = [market_data[t]["spot"] for t in tickers]
    
    # Use default values for calculations
    rates = [0.064] * len(tickers)  # 6.4% default rate
    times = [30/365] * len(tickers)  # 30 days to expiry
    
    # Perform batch calculations
    calc_results = OptimizedFinancialCalculations.calculate_theoretical_values_batch(
        spots, rates, times
    )
    
    # Enrich data with calculations
    enriched_data = []
    for i, ticker in enumerate(tickers):
        stock = market_data[ticker].copy()
        
        # Add calculated values
        stock["calculations"] = {
            "theoretical_value": round(calc_results["theoretical_values"][i], 2),
            "one_sdv": round(calc_results["one_sdv"][i], 2),
            "two_sdv": round(calc_results["two_sdv"][i], 2),
            "three_sdv": round(calc_results["three_sdv"][i], 2),
            "mean_percent": round(np.random.uniform(25, 30), 1),
            "act_difference": round(np.random.uniform(-1, 1), 3),
            "percentage_over_cash": round(np.random.uniform(-2, 2), 3),
            "futures_cash_diff": round(np.random.uniform(-5, 5), 2)
        }
        
        # Update futures price with theoretical value
        stock["futures"]["price"] = stock["calculations"]["theoretical_value"]
        stock["futures"]["bid"] = stock["calculations"]["theoretical_value"] - 0.5
        stock["futures"]["ask"] = stock["calculations"]["theoretical_value"] + 0.5
        
        enriched_data.append(stock)
    
    return enriched_data

@app.get("/api/market-data/{ticker}")
async def get_ticker_data(ticker: str):
    ticker = ticker.upper()
    if ticker not in market_data:
        raise HTTPException(status_code=404, detail="Ticker not found")
    return market_data[ticker]

@app.post("/api/market-data/{ticker}")
async def update_ticker_data(ticker: str, data: dict):
    ticker = ticker.upper()
    
    if ticker in market_data:
        market_data[ticker].update(data)
    else:
        # Create new ticker entry
        market_data[ticker] = {
            "ticker": ticker,
            "spot": 0,
            "volume": 0,
            "exchanges": ["NSE"],
            "futures": {"price": 0, "volume": 0, "oi": 0, "expiry": "", "bid": 0, "ask": 0},
            "options": {
                "calls": {"bid": 0, "ask": 0, "ltp": 0, "volume": 0, "oi": 0},
                "puts": {"bid": 0, "ask": 0, "ltp": 0, "volume": 0, "oi": 0}
            },
            "dividends": {"announced": False, "ex_date": None, "amount": 0},
            **data
        }
    
    return market_data[ticker]

@app.get("/api/baskets")
async def get_baskets():
    return baskets

@app.post("/api/baskets")
async def create_basket(basket: BasketData):
    baskets[basket.name] = {
        "name": basket.name,
        "description": basket.description,
        "stocks": basket.stocks,
        "weightages": basket.weightages,
        "created": datetime.now().isoformat()
    }
    return baskets[basket.name]

@app.delete("/api/baskets/{name}")
async def delete_basket(name: str):
    if name in baskets:
        del baskets[name]
        return {"message": "Basket deleted"}
    raise HTTPException(status_code=404, detail="Basket not found")

@app.get("/api/bloomberg/{ticker}")
async def get_bloomberg_data(ticker: str):
    """Get data from Bloomberg API or fallback source"""
    data = await fetch_bloomberg_data(ticker)
    return data

# WebSocket for real-time updates
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    active_connections.append(websocket)
    
    try:
        # Send initial data
        initial_data = await get_market_data()
        await websocket.send_text(json.dumps({
            "type": "INITIAL_DATA",
            "data": initial_data
        }))
        
        # Keep connection alive and send periodic updates
        while True:
            await asyncio.sleep(5)  # Update every 5 seconds
            
            # Simulate price changes with numpy for speed
            for ticker in market_data:
                change = np.random.normal(0, 1)  # Random walk
                market_data[ticker]["spot"] += change
                market_data[ticker]["spot"] = max(1, market_data[ticker]["spot"])  # Ensure positive
            
            updated_data = await get_market_data()
            await websocket.send_text(json.dumps({
                "type": "MARKET_UPDATE",
                "data": updated_data,
                "timestamp": datetime.now().isoformat()
            }))
            
    except Exception as e:
        logger.error(f"WebSocket error: {e}")
    finally:
        active_connections.remove(websocket)

# Background task for continuous data updates
async def update_market_data():
    """Background task to continuously update market data"""
    while True:
        try:
            # Fetch real-time data for all tickers
            for ticker in list(market_data.keys()):
                new_data = await fetch_bloomberg_data(ticker)
                if new_data:
                    market_data[ticker].update(new_data)
            
            # Broadcast updates to all connected clients
            if active_connections:
                updated_data = await get_market_data()
                message = json.dumps({
                    "type": "MARKET_UPDATE",
                    "data": updated_data,
                    "timestamp": datetime.now().isoformat()
                })
                
                # Send to all connected clients
                for connection in active_connections.copy():
                    try:
                        await connection.send_text(message)
                    except:
                        active_connections.remove(connection)
            
            await asyncio.sleep(1)  # Update every second
            
        except Exception as e:
            logger.error(f"Background update error: {e}")
            await asyncio.sleep(5)

@app.on_event("startup")
async def startup_event():
    """Initialize data and start background tasks"""
    initialize_sample_data()
    
    # Start background data update task
    asyncio.create_task(update_market_data())

if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=5000,
        reload=True,
        log_level="info"
    )