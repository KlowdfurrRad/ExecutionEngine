import yfinance as yf
import pandas as pd

# 1. Create a Ticker object for the stock (e.g., Apple Inc., ticker 'AAPL')
aapl = yf.Ticker('AAPL')

# 2. Get the list of available expiration dates
# This returns a tuple of strings representing the dates
expiration_dates = aapl.options
print("Available Expiration Dates:", expiration_dates)

# 3. Choose one of the expiration dates to download the options chain
# For this example, we'll use the first available date
first_expiry = expiration_dates[0]
print("Fetching options chain for:", first_expiry)

# 4. Download the options chain for that specific expiration date
# This returns a tuple of two pandas DataFrames: one for calls and one for puts
options_chain = aapl.option_chain(first_expiry)

# 5. Access and print the call options data
calls = options_chain.calls
print("\nCall Options:")
print(calls.head(50))
print(calls.columns)

# 6. Access and print the put options data
puts = options_chain.puts
print("\nPut Options:")
print(puts.head())
