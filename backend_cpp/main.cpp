#include <crow.h>
#include <crow/middlewares/cors.h>
#include <nlohmann/json.hpp>
#include <cmath>
#include <random>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;
using namespace std;

// Financial calculation class with optimized algorithms
class FinancialCalculator {
private:
    // Fast inverse normal CDF approximation
    static double inverseNormalCDF(double p) {
        if (p <= 0.0 || p >= 1.0) return 0.0;
        
        double x = p - 0.5;
        if (abs(x) < 0.42) {
            double r = x * x;
            return x * ((((-25.44106049637 * r + 41.39119773534) * r 
                        - 18.61500062529) * r + 2.50662823884) * r - 0.25449438889)
                    / ((((3.13082909833 * r - 21.06224101826) * r 
                        + 23.08336743743) * r - 8.47351093090) * r + 1.0);
        }
        
        double r = (x > 0) ? 1.0 - p : p;
        r = sqrt(-log(r));
        
        double result;
        if (r < 5.0) {
            r -= 1.6;
            result = (((((0.007540014 * r + 0.032776263) * r + 0.104046394) * r 
                     + 0.178644675) * r + 0.174420264) * r + 1.0) * r / 
                    (((((0.015995094 * r + 0.154943985) * r + 0.544459638) * r 
                     + 1.0) * r + 1.0) * r + 1.0);
        } else {
            r -= 5.0;
            result = (((((0.000454004 * r + 0.010009993) * r + 0.043534261) * r 
                     + 0.100012428) * r + 0.182741207) * r + 1.0) * r / 
                    (((((0.001076324 * r + 0.013734698) * r + 0.089433576) * r 
                     + 0.285324221) * r + 1.0) * r + 1.0);
        }
        
        return (x > 0) ? result : -result;
    }

    // Normal CDF approximation using Hart's algorithm
    static double normalCDF(double x) {
        double a1 = 0.254829592;
        double a2 = -0.284496736;
        double a3 = 1.421413741;
        double a4 = -1.453152027;
        double a5 = 1.061405429;
        double p = 0.3275911;
        
        int sign = (x >= 0) ? 1 : -1;
        x = abs(x);
        
        double t = 1.0 / (1.0 + p * x);
        double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);
        
        return 0.5 * (1.0 + sign * y);
    }

public:
    // High-performance Black-Scholes calculation
    static double blackScholesCall(double S, double K, double r, double T, double sigma) {
        if (T <= 0) return max(S - K, 0.0);
        
        double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrt(T));
        double d2 = d1 - sigma * sqrt(T);
        
        return S * normalCDF(d1) - K * exp(-r * T) * normalCDF(d2);
    }
    
    static double blackScholesPut(double S, double K, double r, double T, double sigma) {
        if (T <= 0) return max(K - S, 0.0);
        
        double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrt(T));
        double d2 = d1 - sigma * sqrt(T);
        
        return K * exp(-r * T) * normalCDF(-d2) - S * normalCDF(-d1);
    }
    
    // Vectorized calculations for multiple instruments
    static json calculateBatchMetrics(const vector<double>& spots, const vector<double>& rates, 
                                    const vector<double>& times, const vector<double>& volatilities) {
        json result;
        result["theoretical_values"] = json::array();
        result["one_sdv"] = json::array();
        result["two_sdv"] = json::array();
        result["three_sdv"] = json::array();
        result["call_prices"] = json::array();
        result["put_prices"] = json::array();
        
        for (size_t i = 0; i < spots.size(); ++i) {
            double spot = spots[i];
            double rate = rates[i % rates.size()];
            double time = times[i % times.size()];
            double vol = volatilities[i % volatilities.size()];
            
            // Theoretical future value
            double theoretical = spot * exp(rate * time);
            result["theoretical_values"].push_back(round(theoretical * 100) / 100);
            
            // SDV levels
            double sdv = spot * vol * sqrt(time);
            result["one_sdv"].push_back(round(sdv * 100) / 100);
            result["two_sdv"].push_back(round(2 * sdv * 100) / 100);
            result["three_sdv"].push_back(round(3 * sdv * 100) / 100);
            
            // Option prices (ATM strike)
            double call_price = blackScholesCall(spot, spot, rate, time, vol);
            double put_price = blackScholesPut(spot, spot, rate, time, vol);
            result["call_prices"].push_back(round(call_price * 100) / 100);
            result["put_prices"].push_back(round(put_price * 100) / 100);
        }
        
        return result;
    }
    
    // Monte Carlo simulation for complex instruments
    static double monteCarloOptionPrice(double S, double K, double r, double T, double sigma, 
                                      int simulations = 100000, bool isCall = true) {
        random_device rd;
        mt19937 gen(rd());
        normal_distribution<> d(0, 1);
        
        double sum = 0.0;
        double dt = T;
        double drift = (r - 0.5 * sigma * sigma) * dt;
        double diffusion = sigma * sqrt(dt);
        
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < simulations; ++i) {
            double ST = S * exp(drift + diffusion * d(gen));
            double payoff = isCall ? max(ST - K, 0.0) : max(K - ST, 0.0);
            sum += payoff;
        }
        
        return exp(-r * T) * sum / simulations;
    }
};

// Global data storage
map<string, json> marketData;
map<string, json> baskets;
vector<crow::websocket::connection*> wsConnections;

// Initialize sample market data
void initializeMarketData() {
    vector<string> tickers = {"HDFCBANK", "AXISBANK", "RELIANCE", "TCS", "INFY", "ICICIBANK", "SBIN", "WIPRO", "LT", "BAJFINANCE"};
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> price_dist(400.0, 600.0);
    uniform_int_distribution<> volume_dist(10000, 100000);
    uniform_real_distribution<> option_dist(15.0, 45.0);
    
    for (const auto& ticker : tickers) {
        double spot = round(price_dist(gen) * 100) / 100;
        
        marketData[ticker] = {
            {"ticker", ticker},
            {"spot", spot},
            {"volume", volume_dist(gen)},
            {"exchanges", {"NSE", "BSE"}},
            {"futures", {
                {"price", round((spot * 1.01) * 100) / 100},
                {"volume", volume_dist(gen) / 2},
                {"oi", volume_dist(gen) * 3},
                {"expiry", "28NOV25"},
                {"bid", round((spot * 0.995) * 100) / 100},
                {"ask", round((spot * 1.005) * 100) / 100}
            }},
            {"options", {
                {"calls", {
                    {"bid", round(option_dist(gen) * 100) / 100},
                    {"ask", round(option_dist(gen) * 100) / 100},
                    {"ltp", round(option_dist(gen) * 100) / 100},
                    {"volume", volume_dist(gen) / 10},
                    {"oi", volume_dist(gen) / 2}
                }},
                {"puts", {
                    {"bid", round(option_dist(gen) * 100) / 100},
                    {"ask", round(option_dist(gen) * 100) / 100},
                    {"ltp", round(option_dist(gen) * 100) / 100},
                    {"volume", volume_dist(gen) / 10},
                    {"oi", volume_dist(gen) / 2}
                }}
            }},
            {"dividends", {
                {"announced", false},
                {"ex_date", nullptr},
                {"amount", 0}
            }}
        };
    }
}

// Enhanced market data with calculations
json getEnrichedMarketData() {
    vector<double> spots, rates, times, vols;
    
    for (const auto& [ticker, data] : marketData) {
        spots.push_back(data["spot"]);
        rates.push_back(0.064); // 6.4% default
        times.push_back(30.0 / 365.0); // 30 days
        vols.push_back(0.25); // 25% volatility
    }
    
    auto calculations = FinancialCalculator::calculateBatchMetrics(spots, rates, times, vols);
    
    json enrichedData = json::array();
    int i = 0;
    
    for (const auto& [ticker, data] : marketData) {
        json enriched = data;
        
        enriched["calculations"] = {
            {"theoretical_value", calculations["theoretical_values"][i]},
            {"one_sdv", calculations["one_sdv"][i]},
            {"two_sdv", calculations["two_sdv"][i]},
            {"three_sdv", calculations["three_sdv"][i]},
            {"call_price", calculations["call_prices"][i]},
            {"put_price", calculations["put_prices"][i]},
            {"mean_percent", round(uniform_real_distribution<>(25.0, 30.0)(mt19937{random_device{}()}) * 10) / 10},
            {"act_difference", round(uniform_real_distribution<>(-1.0, 1.0)(mt19937{random_device{}()}) * 1000) / 1000},
            {"percentage_over_cash", round(((enriched["futures"]["price"].get<double>() - enriched["spot"].get<double>()) / enriched["spot"].get<double>() * 100) * 1000) / 1000},
            {"futures_cash_diff", round((enriched["futures"]["price"].get<double>() - enriched["spot"].get<double>()) * 100) / 100}
        };
        
        enrichedData.push_back(enriched);
        i++;
    }
    
    return enrichedData;
}

// WebSocket message broadcaster
void broadcastMarketUpdate() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(3));
        
        // Update prices with random walk
        random_device rd;
        mt19937 gen(rd());
        normal_distribution<> d(0, 1.0);
        
        for (auto& [ticker, data] : marketData) {
            double currentSpot = data["spot"];
            double change = d(gen);
            double newSpot = max(1.0, currentSpot + change);
            data["spot"] = round(newSpot * 100) / 100;
            
            // Update futures price
            data["futures"]["price"] = round((newSpot * 1.01) * 100) / 100;
            data["futures"]["bid"] = round((newSpot * 0.995) * 100) / 100;
            data["futures"]["ask"] = round((newSpot * 1.005) * 100) / 100;
        }
        
        if (!wsConnections.empty()) {
            json update = {
                {"type", "MARKET_UPDATE"},
                {"data", getEnrichedMarketData()},
                {"timestamp", chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()}
            };
            
            string message = update.dump();
            
            // Broadcast to all connected clients
            for (auto* conn : wsConnections) {
                if (conn) {
                    conn->send_text(message);
                }
            }
        }
    }
}

int main() {
    // Initialize data
    initializeMarketData();
    
    // Start background thread for market updates
    thread marketThread(broadcastMarketUpdate);
    marketThread.detach();
    
    // Create Crow app with CORS
    crow::App<crow::CORSHandler> app;
    
    // Configure CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .headers("Content-Type", "Authorization")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .prefix("/api")
        .origin("http://localhost:3000");
    
    // Root endpoint
    CROW_ROUTE(app, "/")([](){
        return json{
            {"message", "Cash Futures THV API - C++ Crow Backend"},
            {"version", "1.0.0"},
            {"performance", "Ultra High Speed"},
            {"features", {"WebSocket", "REST API", "Real-time calculations"}}
        }.dump();
    });
    
    // Configuration endpoint
    CROW_ROUTE(app, "/api/config")([](){
        return json{
            {"interest_rates", {
                {"7", 6.2}, {"30", 6.4}, {"60", 6.7}, {"90", 6.9}, {"180", 7.1}
            }},
            {"expiries", {7, 30, 60, 90, 180, 365}},
            {"exchanges", {"NSE", "BSE", "MCX", "NCDEX"}},
            {"vix_enabled", true}
        }.dump();
    });
    
    // Get all market data
    CROW_ROUTE(app, "/api/market-data").methods("GET"_method)([](){
        return getEnrichedMarketData().dump();
    });
    
    // Get specific ticker
    CROW_ROUTE(app, "/api/market-data/<string>").methods("GET"_method)([](const string& ticker){
        string upperTicker = ticker;
        transform(upperTicker.begin(), upperTicker.end(), upperTicker.begin(), ::toupper);
        
        if (marketData.find(upperTicker) != marketData.end()) {
            return crow::response(200, marketData[upperTicker].dump());
        }
        return crow::response(404, json{{"error", "Ticker not found"}}.dump());
    });
    
    // Update/Create ticker
    CROW_ROUTE(app, "/api/market-data/<string>").methods("POST"_method)([](const crow::request& req, const string& ticker){
        string upperTicker = ticker;
        transform(upperTicker.begin(), upperTicker.end(), upperTicker.begin(), ::toupper);
        
        try {
            json requestData = json::parse(req.body);
            
            if (marketData.find(upperTicker) != marketData.end()) {
                // Update existing
                marketData[upperTicker].update(requestData);
            } else {
                // Create new
                marketData[upperTicker] = {
                    {"ticker", upperTicker},
                    {"spot", 0},
                    {"volume", 0},
                    {"exchanges", {"NSE"}},
                    {"futures", {{"price", 0}, {"volume", 0}, {"oi", 0}, {"expiry", ""}, {"bid", 0}, {"ask", 0}}},
                    {"options", {
                        {"calls", {{"bid", 0}, {"ask", 0}, {"ltp", 0}, {"volume", 0}, {"oi", 0}}},
                        {"puts", {{"bid", 0}, {"ask", 0}, {"ltp", 0}, {"volume", 0}, {"oi", 0}}}
                    }},
                    {"dividends", {{"announced", false}, {"ex_date", nullptr}, {"amount", 0}}}
                };
                marketData[upperTicker].update(requestData);
            }
            
            return crow::response(200, marketData[upperTicker].dump());
        } catch (const exception& e) {
            return crow::response(400, json{{"error", "Invalid JSON"}}.dump());
        }
    });
    
    // Baskets management
    CROW_ROUTE(app, "/api/baskets").methods("GET"_method)([](){
        return json(baskets).dump();
    });
    
    CROW_ROUTE(app, "/api/baskets").methods("POST"_method)([](const crow::request& req){
        try {
            json basketData = json::parse(req.body);
            string name = basketData["name"];
            
            baskets[name] = {
                {"name", name},
                {"description", basketData.value("description", "")},
                {"stocks", basketData["stocks"]},
                {"weightages", basketData["weightages"]},
                {"created", chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()}
            };
            
            return crow::response(201, baskets[name].dump());
        } catch (const exception& e) {
            return crow::response(400, json{{"error", "Invalid basket data"}}.dump());
        }
    });
    
    CROW_ROUTE(app, "/api/baskets/<string>").methods("DELETE"_method)([](const string& name){
        if (baskets.find(name) != baskets.end()) {
            baskets.erase(name);
            return crow::response(200, json{{"message", "Basket deleted"}}.dump());
        }
        return crow::response(404, json{{"error", "Basket not found"}}.dump());
    });
    
    // WebSocket endpoint
    CROW_ROUTE(app, "/ws").websocket()
        .onopen([&](crow::websocket::connection& conn){
            wsConnections.push_back(&conn);
            cout << "WebSocket client connected. Total clients: " << wsConnections.size() << endl;
            
            // Send initial data
            json initialData = {
                {"type", "INITIAL_DATA"},
                {"data", getEnrichedMarketData()}
            };
            conn.send_text(initialData.dump());
        })
        .onclose([&](crow::websocket::connection& conn, const string& reason){
            wsConnections.erase(remove(wsConnections.begin(), wsConnections.end(), &conn), wsConnections.end());
            cout << "WebSocket client disconnected. Total clients: " << wsConnections.size() << endl;
        })
        .onmessage([](crow::websocket::connection& /*conn*/, const string& data, bool /*is_binary*/){
            cout << "Received WebSocket message: " << data << endl;
        });
    
    cout << "==================================================" << endl;
    cout << "🚀 C++ High-Performance Backend Starting..." << endl;
    cout << "==================================================" << endl;
    cout << "Server: http://localhost:5002" << endl;
    cout << "WebSocket: ws://localhost:5002/ws" << endl;
    cout << "Market Data: " << marketData.size() << " instruments loaded" << endl;
    cout << "Features: Real-time calculations, WebSocket, REST API" << endl;
    cout << "==================================================" << endl;
    
    app.port(5002).multithreaded().run();
    
    return 0;
}