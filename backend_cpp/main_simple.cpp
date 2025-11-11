// Simple C++ Backend for Cash Futures THV
// Uses only standard libraries for easy compilation
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>
#include <random>
#include <thread>
#include <chrono>
#include <fstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

using namespace std;

// Simple JSON-like structure for data exchange
class SimpleJSON {
private:
    map<string, string> data;
    
public:
    void set(const string& key, const string& value) {
        data[key] = value;
    }
    
    void set(const string& key, double value) {
        data[key] = to_string(value);
    }
    
    void set(const string& key, int value) {
        data[key] = to_string(value);
    }
    
    string get(const string& key) const {
        auto it = data.find(key);
        return (it != data.end()) ? it->second : "";
    }
    
    string toJSON() const {
        stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& pair : data) {
            if (!first) ss << ",";
            ss << "\"" << pair.first << "\":\"" << pair.second << "\"";
            first = false;
        }
        ss << "}";
        return ss.str();
    }
};

// Financial calculations using standard library only
class FinancialCalc {
public:
    // Normal CDF approximation
    static double normalCDF(double x) {
        return 0.5 * erfc(-x * M_SQRT1_2);
    }
    
    // Black-Scholes call option price
    static double blackScholesCall(double S, double K, double r, double T, double sigma) {
        if (T <= 0) return max(S - K, 0.0);
        
        double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrt(T));
        double d2 = d1 - sigma * sqrt(T);
        
        return S * normalCDF(d1) - K * exp(-r * T) * normalCDF(d2);
    }
    
    // Black-Scholes put option price
    static double blackScholesPut(double S, double K, double r, double T, double sigma) {
        if (T <= 0) return max(K - S, 0.0);
        
        double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrt(T));
        double d2 = d1 - sigma * sqrt(T);
        
        return K * exp(-r * T) * normalCDF(-d2) - S * normalCDF(-d1);
    }
    
    // Calculate theoretical future value
    static double theoreticalValue(double spot, double rate, double time) {
        return spot * exp(rate * time);
    }
    
    // Calculate Standard Deviation Value
    static double calculateSDV(double spot, double volatility, double time) {
        return spot * volatility * sqrt(time);
    }
};

// Simple market data structure
struct MarketData {
    string ticker;
    double spot;
    int volume;
    double futuresPrice;
    int futuresVolume;
    string expiry;
    double bid;
    double ask;
    
    MarketData(const string& t = "", double s = 0.0) : 
        ticker(t), spot(s), volume(0), futuresPrice(s * 1.01), 
        futuresVolume(0), expiry("28NOV25"), bid(s * 0.995), ask(s * 1.005) {}
};

// Global market data storage
map<string, MarketData> marketData;

// Initialize sample data
void initializeData() {
    vector<string> tickers = {"HDFCBANK", "AXISBANK", "RELIANCE", "TCS", "INFY"};
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> price_dist(400.0, 600.0);
    uniform_int_distribution<> volume_dist(10000, 100000);
    
    for (const auto& ticker : tickers) {
        double price = round(price_dist(gen) * 100) / 100;
        MarketData data(ticker, price);
        data.volume = volume_dist(gen);
        data.futuresVolume = volume_dist(gen) / 2;
        marketData[ticker] = data;
    }
}

// Generate market data JSON response
string getMarketDataJSON() {
    stringstream ss;
    ss << "[";
    bool first = true;
    
    for (const auto& pair : marketData) {
        if (!first) ss << ",";
        const auto& data = pair.second;
        
        // Calculate values
        double rate = 0.064; // 6.4%
        double time = 30.0 / 365.0; // 30 days
        double vol = 0.25; // 25%
        
        double theoretical = FinancialCalc::theoreticalValue(data.spot, rate, time);
        double oneSDV = FinancialCalc::calculateSDV(data.spot, vol, time);
        double callPrice = FinancialCalc::blackScholesCall(data.spot, data.spot, rate, time, vol);
        double putPrice = FinancialCalc::blackScholesPut(data.spot, data.spot, rate, time, vol);
        double percentOverCash = ((data.futuresPrice - data.spot) / data.spot) * 100;
        
        ss << "{"
           << "\"ticker\":\"" << data.ticker << "\","
           << "\"spot\":" << data.spot << ","
           << "\"volume\":" << data.volume << ","
           << "\"futures\":{\"price\":" << data.futuresPrice << ","
           << "\"volume\":" << data.futuresVolume << ","
           << "\"expiry\":\"" << data.expiry << "\","
           << "\"bid\":" << data.bid << ","
           << "\"ask\":" << data.ask << "},"
           << "\"calculations\":{"
           << "\"theoreticalValue\":" << round(theoretical * 100) / 100 << ","
           << "\"oneSDV\":" << round(oneSDV * 100) / 100 << ","
           << "\"twoSDV\":" << round(oneSDV * 2 * 100) / 100 << ","
           << "\"threeSDV\":" << round(oneSDV * 3 * 100) / 100 << ","
           << "\"callPrice\":" << round(callPrice * 100) / 100 << ","
           << "\"putPrice\":" << round(putPrice * 100) / 100 << ","
           << "\"percentageOverCash\":" << round(percentOverCash * 1000) / 1000 << ","
           << "\"futuresCashDiff\":" << round((data.futuresPrice - data.spot) * 100) / 100 << ","
           << "\"meanPercent\":27.5,"
           << "\"actDifference\":-0.063"
           << "}}";
        
        first = false;
    }
    ss << "]";
    return ss.str();
}

// Simple HTTP response
string createHTTPResponse(const string& content, const string& contentType = "application/json") {
    stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "\r\n";
    response << content;
    return response.str();
}

// Handle HTTP requests
string handleRequest(const string& request) {
    if (request.find("GET /api/market-data") != string::npos) {
        return createHTTPResponse(getMarketDataJSON());
    } else if (request.find("GET /api/config") != string::npos) {
        string config = R"({"interest_rates":{"7":6.2,"30":6.4,"60":6.7,"90":6.9,"180":7.1},"expiries":[7,30,60,90,180,365],"exchanges":["NSE","BSE","MCX","NCDEX"],"vix_enabled":true})";
        return createHTTPResponse(config);
    } else if (request.find("GET /api/baskets") != string::npos) {
        return createHTTPResponse("{}");
    } else if (request.find("GET /") != string::npos) {
        string info = R"({"message":"Cash Futures THV API - C++ Simple Backend","version":"1.0.0","performance":"High Speed","note":"Standard libraries only - no external dependencies"})";
        return createHTTPResponse(info);
    } else if (request.find("OPTIONS") != string::npos) {
        return createHTTPResponse("", "text/plain");
    }
    
    return createHTTPResponse(R"({"error":"Not found"})", "application/json");
}

// Simple socket server
void runServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Socket creation failed" << endl;
        return;
    }
    
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        return;
    }
    
    if (listen(server_fd, 10) < 0) {
        cerr << "Listen failed" << endl;
        return;
    }
    
    cout << "=================================================" << endl;
    cout << "🚀 C++ Simple Backend Started!" << endl;
    cout << "=================================================" << endl;
    cout << "📡 Server: http://localhost:" << port << endl;
    cout << "📊 Market Data: " << marketData.size() << " instruments loaded" << endl;
    cout << "💻 Standard libraries only - no dependencies!" << endl;
    cout << "=================================================" << endl;
    
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
#ifdef _WIN32
        SOCKET client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd == INVALID_SOCKET) continue;
#else
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) continue;
#endif
        
        char buffer[4096] = {0};
        recv(client_fd, buffer, sizeof(buffer), 0);
        
        string request(buffer);
        string response = handleRequest(request);
        
        send(client_fd, response.c_str(), response.length(), 0);
        
#ifdef _WIN32
        closesocket(client_fd);
#else
        close(client_fd);
#endif
    }
    
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
}

// Background market data updater
void updateMarketData() {
    random_device rd;
    mt19937 gen(rd());
    normal_distribution<> price_change(0, 1.0);
    
    while (true) {
        this_thread::sleep_for(chrono::seconds(5));
        
        for (auto& pair : marketData) {
            auto& data = pair.second;
            double change = price_change(gen);
            data.spot = max(1.0, data.spot + change);
            data.futuresPrice = data.spot * 1.01;
            data.bid = data.spot * 0.995;
            data.ask = data.spot * 1.005;
        }
    }
}

int main() {
    cout << "Initializing Cash Futures THV C++ Backend..." << endl;
    
    initializeData();
    
    // Start background market data updater
    thread updater(updateMarketData);
    updater.detach();
    
    // Start HTTP server
    runServer(8002);
    
    return 0;
}