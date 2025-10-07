#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <map>
#include <curl/curl.h>
#include "pricing_engine.h"

namespace trading {

struct ZerodhaCredentials {
    std::string api_key;
    std::string api_secret;
    std::string access_token;
    std::string user_id;
};

struct InstrumentToken {
    std::string instrument_token;
    std::string exchange_token;
    std::string tradingsymbol;
    std::string name;
    std::string exchange;
    std::string segment;
    std::string instrument_type;
    double tick_size;
    double lot_size;
    std::chrono::system_clock::time_point expiry;
    double strike;
};

struct Quote {
    std::string instrument_token;
    double last_price;
    double ohlc_open;
    double ohlc_high;
    double ohlc_low;
    double ohlc_close;
    double change;
    double change_percent;
    uint64_t volume;
    uint64_t average_price;
    uint64_t oi;
    double bid_price;
    double ask_price;
    uint64_t bid_quantity;
    uint64_t ask_quantity;
    std::chrono::system_clock::time_point last_trade_time;
    std::chrono::system_clock::time_point timestamp;
};

struct OrderResponse {
    std::string order_id;
    std::string status;
    std::string message;
    std::string rejection_reason;
};

struct Position {
    std::string tradingsymbol;
    std::string exchange;
    std::string instrument_token;
    std::string product;
    int quantity;
    int overnight_quantity;
    int multiplier;
    double average_price;
    double close_price;
    double last_price;
    double value;
    double pnl;
    double m2m;
    double unrealised;
    double realised;
};

struct TickData {
    std::string instrument_token;
    double last_price;
    uint64_t last_traded_quantity;
    double average_traded_price;
    uint64_t volume_traded;
    uint64_t total_buy_quantity;
    uint64_t total_sell_quantity;
    double ohlc_open;
    double ohlc_high;
    double ohlc_low;
    double ohlc_close;
    double net_change;
    uint64_t oi;
    std::chrono::system_clock::time_point timestamp;
    std::vector<std::pair<double, uint64_t>> market_depth_buy;
    std::vector<std::pair<double, uint64_t>> market_depth_sell;
};

class ZerodhaConnector {
public:
    ZerodhaConnector(const ZerodhaCredentials& credentials);
    ~ZerodhaConnector();

    // Authentication
    bool authenticate();
    bool isAuthenticated() const { return authenticated_; }
    void setAccessToken(const std::string& token);

    // Market data
    Quote getQuote(const std::string& instrument_token);
    std::vector<Quote> getQuotes(const std::vector<std::string>& instrument_tokens);
    std::vector<InstrumentToken> getInstruments(const std::string& exchange = "");
    
    // Historical data
    struct HistoricalData {
        std::chrono::system_clock::time_point date;
        double open;
        double high;
        double low;
        double close;
        uint64_t volume;
        uint64_t oi;
    };
    
    std::vector<HistoricalData> getHistoricalData(
        const std::string& instrument_token,
        const std::string& interval,
        const std::chrono::system_clock::time_point& from_date,
        const std::chrono::system_clock::time_point& to_date
    );

    // Trading
    OrderResponse placeOrder(
        const std::string& tradingsymbol,
        const std::string& exchange,
        const std::string& transaction_type, // BUY/SELL
        int quantity,
        const std::string& product,          // MIS/CNC/NRML
        const std::string& order_type,       // MARKET/LIMIT
        double price = 0.0,
        const std::string& validity = "DAY",
        const std::string& variety = "regular"
    );
    
    OrderResponse modifyOrder(
        const std::string& order_id,
        int quantity,
        double price,
        const std::string& order_type,
        const std::string& validity = "DAY"
    );
    
    OrderResponse cancelOrder(const std::string& order_id, const std::string& variety = "regular");
    
    // Portfolio
    std::vector<Position> getPositions();
    double getMargins(const std::string& segment = "equity");
    
    // WebSocket for real-time data
    bool connectWebSocket();
    void disconnectWebSocket();
    bool subscribeToTicks(const std::vector<std::string>& instrument_tokens);
    bool unsubscribeFromTicks(const std::vector<std::string>& instrument_tokens);
    
    // Callback for tick data
    void setTickCallback(std::function<void(const TickData&)> callback);
    
    // Rate limiting and error handling
    void setRateLimit(int requests_per_second = 10);
    std::string getLastError() const { return last_error_; }

private:
    // HTTP request handling
    struct HTTPResponse {
        std::string data;
        long response_code;
        std::string error;
    };
    
    HTTPResponse makeRequest(
        const std::string& url,
        const std::string& method = "GET",
        const std::string& data = "",
        const std::map<std::string, std::string>& headers = {}
    );
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HTTPResponse* response);
    
    // URL construction
    std::string buildURL(const std::string& endpoint, const std::map<std::string, std::string>& params = {});
    std::string urlEncode(const std::string& value);
    
    // JSON parsing helpers
    Quote parseQuote(const std::string& json_data, const std::string& instrument_token);
    std::vector<Quote> parseQuotes(const std::string& json_data);
    std::vector<InstrumentToken> parseInstruments(const std::string& csv_data);
    OrderResponse parseOrderResponse(const std::string& json_data);
    std::vector<Position> parsePositions(const std::string& json_data);
    std::vector<HistoricalData> parseHistoricalData(const std::string& json_data);
    
    // WebSocket handling
    void websocketThread();
    void handleWebSocketMessage(const std::string& message);
    TickData parseTickData(const std::vector<uint8_t>& binary_data);
    
    // Rate limiting
    void checkRateLimit();
    
    // Configuration and state
    ZerodhaCredentials credentials_;
    bool authenticated_;
    std::string base_url_;
    std::string websocket_url_;
    std::string last_error_;
    
    // HTTP client
    CURL* curl_;
    
    // WebSocket state
    bool websocket_connected_;
    std::vector<std::string> subscribed_tokens_;
    std::function<void(const TickData&)> tick_callback_;
    
    // Rate limiting
    int rate_limit_;
    std::chrono::system_clock::time_point last_request_time_;
    std::vector<std::chrono::system_clock::time_point> request_timestamps_;
    
    // Thread management
    std::thread websocket_thread_;
    std::atomic<bool> stop_websocket_;
    mutable std::mutex mutex_;
};

// Utility class for instrument token management
class InstrumentManager {
public:
    InstrumentManager(std::shared_ptr<ZerodhaConnector> connector);
    
    void loadInstruments();
    std::string getInstrumentToken(const std::string& tradingsymbol, const std::string& exchange);
    InstrumentToken getInstrument(const std::string& tradingsymbol, const std::string& exchange);
    std::vector<InstrumentToken> findOptions(
        const std::string& underlying,
        const std::string& exchange,
        const std::chrono::system_clock::time_point& expiry,
        double strike_min = 0,
        double strike_max = std::numeric_limits<double>::max()
    );
    
    std::vector<InstrumentToken> findFutures(
        const std::string& underlying,
        const std::string& exchange,
        const std::chrono::system_clock::time_point& expiry = {}
    );
    
    bool isMarketOpen();
    std::chrono::system_clock::time_point getNextMarketOpen();

private:
    std::shared_ptr<ZerodhaConnector> connector_;
    std::vector<InstrumentToken> instruments_;
    std::map<std::string, InstrumentToken> symbol_to_instrument_;
    std::chrono::system_clock::time_point last_update_;
    mutable std::mutex mutex_;
};

// Market data feed manager
class MarketDataFeed {
public:
    MarketDataFeed(std::shared_ptr<ZerodhaConnector> connector);
    ~MarketDataFeed();
    
    void start();
    void stop();
    
    void subscribeSymbol(const std::string& symbol, const std::string& exchange);
    void unsubscribeSymbol(const std::string& symbol, const std::string& exchange);
    
    void setMarketDataCallback(std::function<void(const MarketData&)> callback);
    
    MarketData getLastQuote(const std::string& symbol, const std::string& exchange);
    
private:
    void onTickData(const TickData& tick);
    MarketData convertTickToMarketData(const TickData& tick);
    
    std::shared_ptr<ZerodhaConnector> connector_;
    std::shared_ptr<InstrumentManager> instrument_manager_;
    
    std::function<void(const MarketData&)> market_data_callback_;
    std::map<std::string, MarketData> last_quotes_;
    
    bool running_;
    mutable std::mutex mutex_;
};

} // namespace trading