import yfinance as yf
from datetime import datetime, timedelta

# Define the NSE ticker
ticker_symbol = "RELIANCE.NS" 

# Define start and end dates within the 7-day limit for 1-minute interval
end_date = datetime.now()
end_date = end_date - timedelta(days=20)
start_date = end_date - timedelta(days=6) # 6 days to stay within the 7-day limit

# Download 1-minute data for the specified date range
data = yf.download(tickers=ticker_symbol, start=start_date, end=end_date, interval="1m")

print(data)
