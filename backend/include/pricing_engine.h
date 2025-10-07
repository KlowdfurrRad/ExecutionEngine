#pragma once

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>

namespace trading {

struct MarketData {
    std::string symbol;
    double price;
    double bid;
    double ask;
    uint64_t volume;
    uint64_t open_interest;
    double change;
    double change_percent;
    std::chrono::system_clock::time_point last_updated;
    std::string exchange; // "NSE" or "BSE"
};

struct ContractDetails {
    std::string symbol;
    std::string underlying;
    std::string instrument_type; // "CASH", "FUTURE", "OPTION"
    double strike;
    std::chrono::system_clock::time_point expiry;
    std::string option_type; // "CE" or "PE" for options
    double lot_size;
    double tick_size;
};

struct PriceComparison {
    std::string symbol;
    std::string instrument_type;
    double nse_price;
    double bse_price;
    double fair_value;
    double percentage_diff;
    bool is_recommended;
    double liquidity_score;
    uint64_t volume_30day;
    uint64_t current_volume;
    uint64_t open_interest;
    std::string exchange;
    std::chrono::system_clock::time_point expiry_date;
    
    // Additional metrics
    double bid_ask_spread;
    double impact_cost;
    bool volume_compliant; // Within 5% of 30-day average
};

struct OptionGreeks {
    double delta;
    double gamma;
    double theta;
    double vega;
    double rho;
};

struct OptionContract {
    ContractDetails contract;
    MarketData market_data;
    OptionGreeks greeks;
    double implied_volatility;
    double intrinsic_value;
    double time_value;
};

class PricingEngine {
public:
    PricingEngine();
    ~PricingEngine();

    // Core pricing methods
    std::vector<PriceComparison> compareContracts(
        const std::string& underlying,
        double target_quantity = 1.0
    );
    
    PriceComparison findOptimalContract(
        const std::string& underlying,
        const std::string& instrument_type,
        double target_quantity
    );
    
    // Fair value calculations
    double calculateFairValue(const ContractDetails& contract);
    double calculateOptionFairValue(
        double spot_price,
        double strike,
        double time_to_expiry,
        double risk_free_rate,
        double volatility,
        const std::string& option_type
    );
    
    // Liquidity analysis
    double calculateLiquidityScore(const MarketData& data, uint64_t volume_30day);
    bool checkVolumeConstraints(const std::string& symbol, double quantity);
    double calculateImpactCost(const MarketData& data, double quantity);
    
    // Greeks calculations
    OptionGreeks calculateGreeks(
        double spot_price,
        double strike,
        double time_to_expiry,
        double risk_free_rate,
        double volatility,
        const std::string& option_type
    );
    
    // Market data management
    void updateMarketData(const std::vector<MarketData>& data);
    MarketData getMarketData(const std::string& symbol, const std::string& exchange);
    
    // Contract management
    void addContract(const ContractDetails& contract);
    std::vector<ContractDetails> getContracts(const std::string& underlying);
    
    // Historical data for analysis
    void updateHistoricalVolume(const std::string& symbol, uint64_t volume);
    uint64_t get30DayAverageVolume(const std::string& symbol);
    
    // Configuration
    void setRiskFreeRate(double rate) { risk_free_rate_ = rate; }
    void setVolumeThreshold(double threshold) { volume_threshold_ = threshold; }
    
private:
    // Internal calculation methods
    double blackScholes(
        double spot,
        double strike,
        double time_to_expiry,
        double risk_free_rate,
        double volatility,
        bool is_call
    );
    
    double calculateDelta(
        double spot,
        double strike,
        double time_to_expiry,
        double risk_free_rate,
        double volatility,
        bool is_call
    );
    
    double calculateGamma(
        double spot,
        double strike,
        double time_to_expiry,
        double risk_free_rate,
        double volatility
    );
    
    double calculateTheta(
        double spot,
        double strike,
        double time_to_expiry,
        double risk_free_rate,
        double volatility,
        bool is_call
    );
    
    double calculateVega(
        double spot,
        double strike,
        double time_to_expiry,
        double risk_free_rate,
        double volatility
    );
    
    // Utility functions
    double normalCDF(double x);
    double normalPDF(double x);
    double getTimeToExpiry(const std::chrono::system_clock::time_point& expiry);
    std::string selectOptimalExchange(const MarketData& nse_data, const MarketData& bse_data);
    
    // Data storage
    std::map<std::string, MarketData> nse_market_data_;
    std::map<std::string, MarketData> bse_market_data_;
    std::map<std::string, ContractDetails> contracts_;
    std::map<std::string, std::vector<uint64_t>> historical_volumes_; // 30-day rolling window
    
    // Configuration parameters
    double risk_free_rate_;
    double volume_threshold_; // 5% default
    
    // Volatility surface (simplified - in production would be more sophisticated)
    std::map<std::string, double> implied_volatilities_;
};

// Utility class for margin calculations
class MarginCalculator {
public:
    struct MarginRequirement {
        double span_margin;
        double exposure_margin;
        double total_margin;
        std::string calculation_method;
    };
    
    MarginRequirement calculateMargin(
        const std::vector<OptionContract>& positions
    );
    
    double calculateSpanMargin(const OptionContract& contract);
    double calculateExposureMargin(const OptionContract& contract);
    
private:
    // SPAN risk parameters (simplified)
    std::map<std::string, double> price_scan_ranges_;
    std::map<std::string, double> volatility_scan_ranges_;
};

} // namespace trading