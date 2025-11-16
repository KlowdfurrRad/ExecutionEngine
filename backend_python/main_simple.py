from fastapi import FastAPI, WebSocket, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import asyncio
import json
import math
import random
from datetime import datetime
from typing import List, Dict, Optional
from pydantic import BaseModel

app = FastAPI(title="Cash Futures THV API - Python", version="1.0.0")

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

# Global data storage
market_data = {}
baskets = {}
active_connections = []

# Simplified financial calculations without numpy dependency
class SimpleFinancialCalculations:
    
    @staticmethod
    def normal_cdf(x):
        """Approximation of normal cumulative distribution function"""
        # Using Hart approximation
        a1 =  0.254829592
        a2 = -0.284496736
        a3 =  1.421413741
        a4 = -1.453152027
        a5 =  1.061405429
        p  =  0.3275911
        
        sign = 1 if x >= 0 else -1
        x = abs(x)
        
        t = 1.0 / (1.0 + p * x)
        y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * math.exp(-x * x)
        
        return 0.5 * (1.0 + sign * y)
    
    @staticmethod
    def black_scholes_call(S, K, r, T, sigma):
        """Black-Scholes call option pricing"""
        if T <= 0:
            return max(S - K, 0)
        
        d1 = (math.log(S / K) + (r + 0.5 * sigma**2) * T) / (sigma * math.sqrt(T))
        d2 = d1 - sigma * math.sqrt(T)
        
        call_price = S * SimpleFinancialCalculations.normal_cdf(d1) - K * math.exp(-r * T) * SimpleFinancialCalculations.normal_cdf(d2)
        return call_price
    
    @staticmethod
    def black_scholes_put(S, K, r, T, sigma):
        """Black-Scholes put option pricing"""
        if T <= 0:
            return max(K - S, 0)
        
        d1 = (math.log(S / K) + (r + 0.5 * sigma**2) * T) / (sigma * math.sqrt(T))
        d2 = d1 - sigma * math.sqrt(T)
        
        put_price = K * math.exp(-r * T) * SimpleFinancialCalculations.normal_cdf(-d2) - S * SimpleFinancialCalculations.normal_cdf(-d1)
        return put_price
    
    @staticmethod
    def calculate_theoretical_value(spot, rate, time, dividend=0, volatility=0.25):
        """Calculate theoretical futures value"""
        adjusted_spot = spot - dividend
        theoretical_value = adjusted_spot * math.exp(rate * time)
        return round(theoretical_value, 2)
    
    @staticmethod
    def calculate_sdv_levels(spot, volatility, time):
        """Calculate standard deviation levels"""
        sdv_base = spot * volatility * math.sqrt(time)
        return {
            'one_sdv': round(sdv_base, 2),
            'two_sdv': round(2 * sdv_base, 2),
            'three_sdv': round(3 * sdv_base, 2)
        }

# Initialize sample data
def initialize_sample_data():
    global market_data
    
    sample_tickers = ['HDFCBANK', 'AXISBANK', 'RELIANCE', 'TCS', 'INFY', 'ICICIBANK']
    
    for ticker in sample_tickers:
        spot_price = random.uniform(400, 500)
        market_data[ticker] = {
            "ticker": ticker,
            "spot": round(spot_price, 2),
            "volume": random.randint(10000, 100000),
            "exchanges": ["NSE", "BSE"],
            "futures": {
                "price": round(spot_price * 1.01, 2),  # Small premium
                "volume": random.randint(10000, 50000),
                "oi": random.randint(50000, 200000),
                "expiry": "28NOV25",
                "bid": round(spot_price * 0.995, 2),
                "ask": round(spot_price * 1.005, 2)
            },
            "options": {
                "calls": {
                    "bid": round(random.uniform(20, 40), 2),
                    "ask": round(random.uniform(20, 40), 2),
                    "ltp": round(random.uniform(20, 40), 2),
                    "volume": random.randint(1000, 10000),
                    "oi": random.randint(10000, 50000)
                },
                "puts": {
                    "bid": round(random.uniform(20, 40), 2),
                    "ask": round(random.uniform(20, 40), 2),
                    "ltp": round(random.uniform(20, 40), 2),
                    "volume": random.randint(1000, 10000),
                    "oi": random.randint(10000, 50000)
                }
            },
            "dividends": {
                "announced": False,
                "ex_date": None,
                "amount": 0
            }
        }

# API Routes
@app.get("/")
async def root():
    return {"message": "Cash Futures THV API - Python FastAPI Backend", "status": "running"}

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
    """Get all market data with calculations"""
    if not market_data:
        initialize_sample_data()
    
    enriched_data = []
    for ticker, stock in market_data.items():
        # Calculate theoretical values
        rate = 0.064  # 6.4% default rate
        time = 30/365  # 30 days to expiry
        volatility = 0.25  # 25% default volatility
        
        theoretical_value = SimpleFinancialCalculations.calculate_theoretical_value(
            stock["spot"], rate, time, stock["dividends"]["amount"]
        )
        
        sdv_levels = SimpleFinancialCalculations.calculate_sdv_levels(
            stock["spot"], volatility, time
        )
        
        # Add calculated values
        stock_copy = stock.copy()
        stock_copy["calculations"] = {
            "theoretical_value": theoretical_value,
            **sdv_levels,
            "mean_percent": round(random.uniform(25, 30), 1),
            "act_difference": round(random.uniform(-1, 1), 3),
            "percentage_over_cash": round((stock["futures"]["price"] - stock["spot"]) / stock["spot"] * 100, 3),
            "futures_cash_diff": round(stock["futures"]["price"] - stock["spot"], 2)
        }
        
        enriched_data.append(stock_copy)
    
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
            "spot": data.get("spot", 0),
            "volume": data.get("volume", 0),
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
            
            # Simulate price changes
            for ticker in market_data:
                change = random.uniform(-2, 2)  # Random change
                market_data[ticker]["spot"] = max(1, market_data[ticker]["spot"] + change)
                market_data[ticker]["spot"] = round(market_data[ticker]["spot"], 2)
            
            updated_data = await get_market_data()
            await websocket.send_text(json.dumps({
                "type": "MARKET_UPDATE",
                "data": updated_data,
                "timestamp": datetime.now().isoformat()
            }))
            
    except Exception as e:
        print(f"WebSocket error: {e}")
    finally:
        if websocket in active_connections:
            active_connections.remove(websocket)

@app.on_event("startup")
async def startup_event():
    """Initialize data on startup"""
    initialize_sample_data()
    print("Python FastAPI backend started successfully!")
    print("API Documentation: http://localhost:5001/docs")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main_simple:app",
        host="0.0.0.0",
        port=5001,  # Different port to avoid conflict
        reload=True,
        log_level="info"
    )