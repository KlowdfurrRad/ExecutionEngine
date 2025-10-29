import matplotlib.pyplot as plt
import yfinance as yf
from datetime import datetime, timedelta

tickers = ['AAPL', 'MSFT', 'GOOGL', 'AMZN', 'TSLA']
end_date = datetime.now()
start_date = end_date - timedelta(days=365)

data = yf.download(tickers, start=start_date, end=end_date)

for col in data.columns:
    if col[0] == 'Close':
        data['Return ' + col[1]] = data[col].pct_change()

print(data.head())

print("Correlation between returns of AAPL and MSFT:", data['Return AAPL'].corr(data['Return MSFT']))

# Plots MSFT and AAPL returns
plt.figure(figsize=(14, 7))
plt.plot(data['Return AAPL'], label='AAPL Return')
plt.plot(data['Return MSFT'], label='MSFT Return')
plt.title('AAPL vs MSFT Returns Over the Past Year')
plt.xlabel('Date')
plt.ylabel('Return')
plt.legend()
plt.show()

plt.figure(figsize=(10, 6))
plt.scatter(data['Return AAPL'], data['Return MSFT'], alpha=0.5)
plt.title('Scatter Plot of AAPL vs MSFT Returns')
plt.xlabel('AAPL Return')
plt.ylabel('MSFT Return')
plt.grid()
plt.show()

plt.figure(figsize=(10, 6))
plt.scatter(data['Return AAPL'] - data["Return MSFT"],(data['Return AAPL'] - data["Return MSFT"]).shift(-1), alpha=0.5)
plt.title('Scatter Plot of AAPL vs MSFT Returns Difference')
plt.xlabel('AAPL Return - MSFT Return')
plt.ylabel('Next Day Return Difference')
plt.grid()
plt.show()

# https://wire.insiderfinance.io/pair-trading-with-aapl-and-msft-a-statistical-arbitrage-strategy-using-linear-regression-ef484b080654
# https://www.geeksforgeeks.org/python/getting-stock-symbols-with-yfinance-in-python/

# print(historical_data.head())