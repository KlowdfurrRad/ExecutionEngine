#include "pricing_engine.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace trading {

PricingEngine::PricingEngine() 
    : risk_free_rate_(0.06)  // 6% default risk-free rate
    , volume_threshold_(0.05) // 5% volume threshold
{
    // Initialize with some default implied volatilities
    implied_volatilities_["NIFTY"] = 0.15;
    implied_volatilities_["BANKNIFTY"] = 0.18;
    implied_volatilities_["FINNIFTY"] = 0.16;
}

PricingEngine::~PricingEngine() = default;

std::vector<PriceComparison> PricingEngine::compareContracts(
    const std::string& underlying,
    double target_quantity
) {
    std::vector<PriceComparison> comparisons;
    
    // Get all contracts for the underlying
    auto contracts = getContracts(underlying);
    
    for (const auto& contract : contracts) {
        PriceComparison comparison;
        comparison.symbol = contract.symbol;
        comparison.instrument_type = contract.instrument_type;
        
        // Get market data from both exchanges
        auto nse_data = getMarketData(contract.symbol, "NSE");
        auto bse_data = getMarketData(contract.symbol, "BSE");
        
        comparison.nse_price = nse_data.price;
        comparison.bse_price = bse_data.price;
        comparison.fair_value = calculateFairValue(contract);
        
        // Calculate percentage difference from fair value
        double nse_diff = std::abs(nse_data.price - comparison.fair_value) / comparison.fair_value * 100;
        double bse_diff = std::abs(bse_data.price - comparison.fair_value) / comparison.fair_value * 100;
        
        // Choose the exchange with smaller difference
        if (nse_diff < bse_diff) {
            comparison.percentage_diff = (nse_data.price - comparison.fair_value) / comparison.fair_value * 100;
            comparison.exchange = "NSE";
            comparison.current_volume = nse_data.volume;
            comparison.open_interest = nse_data.open_interest;
            comparison.liquidity_score = calculateLiquidityScore(nse_data, get30DayAverageVolume(contract.symbol));
        } else {
            comparison.percentage_diff = (bse_data.price - comparison.fair_value) / comparison.fair_value * 100;
            comparison.exchange = "BSE";
            comparison.current_volume = bse_data.volume;
            comparison.open_interest = bse_data.open_interest;
            comparison.liquidity_score = calculateLiquidityScore(bse_data, get30DayAverageVolume(contract.symbol));
        }
        
        comparison.volume_30day = get30DayAverageVolume(contract.symbol);
        comparison.expiry_date = contract.expiry;
        
        // Check volume compliance
        comparison.volume_compliant = checkVolumeConstraints(contract.symbol, target_quantity);
        
        // Calculate bid-ask spread and impact cost
        auto selected_data = (comparison.exchange == "NSE") ? nse_data : bse_data;
        comparison.bid_ask_spread = (selected_data.ask - selected_data.bid) / selected_data.price * 100;
        comparison.impact_cost = calculateImpactCost(selected_data, target_quantity);
        
        // Recommendation logic
        comparison.is_recommended = (
            std::abs(comparison.percentage_diff) < 0.5 && // Within 0.5% of fair value
            comparison.liquidity_score > 0.6 &&           // Good liquidity
            comparison.volume_compliant &&                // Volume constraint satisfied
            comparison.bid_ask_spread < 1.0               // Tight spread
        );
        
        comparisons.push_back(comparison);
    }
    
    // Sort by absolute percentage difference (best deals first)
    std::sort(comparisons.begin(), comparisons.end(), 
        [](const PriceComparison& a, const PriceComparison& b) {
            return std::abs(a.percentage_diff) < std::abs(b.percentage_diff);
        });
    
    return comparisons;
}

PriceComparison PricingEngine::findOptimalContract(
    const std::string& underlying,
    const std::string& instrument_type,
    double target_quantity
) {
    auto comparisons = compareContracts(underlying, target_quantity);
    
    // Filter by instrument type
    auto it = std::find_if(comparisons.begin(), comparisons.end(),
        [&instrument_type](const PriceComparison& comp) {
            return comp.instrument_type == instrument_type;
        });
    
    if (it != comparisons.end()) {
        return *it;
    }
    
    // Return empty comparison if not found
    return PriceComparison{};
}

double PricingEngine::calculateFairValue(const ContractDetails& contract) {
    if (contract.instrument_type == "CASH") {
        // For cash, fair value is the weighted average of NSE and BSE prices
        auto nse_data = getMarketData(contract.symbol, "NSE");
        auto bse_data = getMarketData(contract.symbol, "BSE");
        
        // Weight by volume
        double total_volume = nse_data.volume + bse_data.volume;
        if (total_volume > 0) {
            return (nse_data.price * nse_data.volume + bse_data.price * bse_data.volume) / total_volume;
        }
        return (nse_data.price + bse_data.price) / 2.0;
        
    } else if (contract.instrument_type == "FUTURE") {
        // For futures, calculate theoretical price using cost of carry model
        auto spot_data = getMarketData(contract.underlying, "NSE");
        double time_to_expiry = getTimeToExpiry(contract.expiry);
        
        // F = S * e^(r * t)
        return spot_data.price * std::exp(risk_free_rate_ * time_to_expiry);
        
    } else if (contract.instrument_type == "OPTION") {
        // For options, use Black-Scholes model
        auto spot_data = getMarketData(contract.underlying, "NSE");
        double time_to_expiry = getTimeToExpiry(contract.expiry);
        double volatility = implied_volatilities_[contract.underlying];
        
        return calculateOptionFairValue(
            spot_data.price,
            contract.strike,
            time_to_expiry,
            risk_free_rate_,
            volatility,
            contract.option_type
        );
    }
    
    return 0.0;
}

double PricingEngine::calculateOptionFairValue(
    double spot_price,
    double strike,
    double time_to_expiry,
    double risk_free_rate,
    double volatility,
    const std::string& option_type
) {
    bool is_call = (option_type == "CE");
    return blackScholes(spot_price, strike, time_to_expiry, risk_free_rate, volatility, is_call);
}

double PricingEngine::blackScholes(
    double spot,
    double strike,
    double time_to_expiry,
    double risk_free_rate,
    double volatility,
    bool is_call
) {
    if (time_to_expiry <= 0) {
        // Option has expired, return intrinsic value
        if (is_call) {
            return std::max(0.0, spot - strike);
        } else {
            return std::max(0.0, strike - spot);
        }
    }
    
    double d1 = (std::log(spot / strike) + (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                / (volatility * std::sqrt(time_to_expiry));
    double d2 = d1 - volatility * std::sqrt(time_to_expiry);
    
    if (is_call) {
        return spot * normalCDF(d1) - strike * std::exp(-risk_free_rate * time_to_expiry) * normalCDF(d2);
    } else {
        return strike * std::exp(-risk_free_rate * time_to_expiry) * normalCDF(-d2) - spot * normalCDF(-d1);
    }
}

OptionGreeks PricingEngine::calculateGreeks(
    double spot_price,
    double strike,
    double time_to_expiry,
    double risk_free_rate,
    double volatility,
    const std::string& option_type
) {
    bool is_call = (option_type == "CE");
    
    OptionGreeks greeks;
    greeks.delta = calculateDelta(spot_price, strike, time_to_expiry, risk_free_rate, volatility, is_call);
    greeks.gamma = calculateGamma(spot_price, strike, time_to_expiry, risk_free_rate, volatility);
    greeks.theta = calculateTheta(spot_price, strike, time_to_expiry, risk_free_rate, volatility, is_call);
    greeks.vega = calculateVega(spot_price, strike, time_to_expiry, risk_free_rate, volatility);
    greeks.rho = 0.0; // Simplified - not implementing rho for now
    
    return greeks;
}

double PricingEngine::calculateDelta(
    double spot,
    double strike,
    double time_to_expiry,
    double risk_free_rate,
    double volatility,
    bool is_call
) {
    if (time_to_expiry <= 0) return 0.0;
    
    double d1 = (std::log(spot / strike) + (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                / (volatility * std::sqrt(time_to_expiry));
    
    if (is_call) {
        return normalCDF(d1);
    } else {
        return normalCDF(d1) - 1.0;
    }
}

double PricingEngine::calculateGamma(
    double spot,
    double strike,
    double time_to_expiry,
    double risk_free_rate,
    double volatility
) {
    if (time_to_expiry <= 0) return 0.0;
    
    double d1 = (std::log(spot / strike) + (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                / (volatility * std::sqrt(time_to_expiry));
    
    return normalPDF(d1) / (spot * volatility * std::sqrt(time_to_expiry));
}

double PricingEngine::calculateTheta(
    double spot,
    double strike,
    double time_to_expiry,
    double risk_free_rate,
    double volatility,
    bool is_call
) {
    if (time_to_expiry <= 0) return 0.0;
    
    double d1 = (std::log(spot / strike) + (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                / (volatility * std::sqrt(time_to_expiry));
    double d2 = d1 - volatility * std::sqrt(time_to_expiry);
    
    double term1 = -(spot * normalPDF(d1) * volatility) / (2 * std::sqrt(time_to_expiry));
    double term2 = risk_free_rate * strike * std::exp(-risk_free_rate * time_to_expiry);
    
    if (is_call) {
        return (term1 - term2 * normalCDF(d2)) / 365.0; // Per day
    } else {
        return (term1 + term2 * normalCDF(-d2)) / 365.0; // Per day
    }
}

double PricingEngine::calculateVega(
    double spot,
    double strike,
    double time_to_expiry,
    double risk_free_rate,
    double volatility
) {
    if (time_to_expiry <= 0) return 0.0;
    
    double d1 = (std::log(spot / strike) + (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                / (volatility * std::sqrt(time_to_expiry));
    
    return spot * normalPDF(d1) * std::sqrt(time_to_expiry) / 100.0; // Per 1% change in volatility
}

double PricingEngine::calculateLiquidityScore(const MarketData& data, uint64_t volume_30day) {
    if (volume_30day == 0) return 0.0;
    
    // Factors: current volume ratio, open interest, bid-ask spread
    double volume_ratio = static_cast<double>(data.volume) / volume_30day;
    double oi_factor = std::min(1.0, static_cast<double>(data.open_interest) / 1000000.0); // Normalize to 1M OI
    double spread_factor = 1.0 / (1.0 + (data.ask - data.bid) / data.price * 100); // Tighter spread = higher score
    
    // Weighted average
    return (volume_ratio * 0.4 + oi_factor * 0.3 + spread_factor * 0.3);
}

bool PricingEngine::checkVolumeConstraints(const std::string& symbol, double quantity) {
    uint64_t avg_volume = get30DayAverageVolume(symbol);
    double threshold_volume = avg_volume * volume_threshold_;
    return quantity <= threshold_volume;
}

double PricingEngine::calculateImpactCost(const MarketData& data, double quantity) {
    // Simplified impact cost model
    // In practice, this would use order book depth
    double volume_ratio = quantity / data.volume;
    return volume_ratio * (data.ask - data.bid) / data.price * 100; // As percentage
}

void PricingEngine::updateMarketData(const std::vector<MarketData>& data) {
    for (const auto& md : data) {
        if (md.exchange == "NSE") {
            nse_market_data_[md.symbol] = md;
        } else if (md.exchange == "BSE") {
            bse_market_data_[md.symbol] = md;
        }
        
        // Update historical volume
        updateHistoricalVolume(md.symbol, md.volume);
    }
}

MarketData PricingEngine::getMarketData(const std::string& symbol, const std::string& exchange) {
    if (exchange == "NSE" && nse_market_data_.find(symbol) != nse_market_data_.end()) {
        return nse_market_data_[symbol];
    } else if (exchange == "BSE" && bse_market_data_.find(symbol) != bse_market_data_.end()) {
        return bse_market_data_[symbol];
    }
    
    // Return empty market data if not found
    MarketData empty_data;
    empty_data.symbol = symbol;
    empty_data.exchange = exchange;
    empty_data.price = 0.0;
    empty_data.bid = 0.0;
    empty_data.ask = 0.0;
    empty_data.volume = 0;
    empty_data.open_interest = 0;
    empty_data.change = 0.0;
    empty_data.change_percent = 0.0;
    empty_data.last_updated = std::chrono::system_clock::now();
    
    return empty_data;
}

void PricingEngine::addContract(const ContractDetails& contract) {
    contracts_[contract.symbol] = contract;
}

std::vector<ContractDetails> PricingEngine::getContracts(const std::string& underlying) {
    std::vector<ContractDetails> result;
    
    for (const auto& pair : contracts_) {
        if (pair.second.underlying == underlying) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

void PricingEngine::updateHistoricalVolume(const std::string& symbol, uint64_t volume) {
    auto& volumes = historical_volumes_[symbol];
    volumes.push_back(volume);
    
    // Keep only last 30 entries
    if (volumes.size() > 30) {
        volumes.erase(volumes.begin());
    }
}

uint64_t PricingEngine::get30DayAverageVolume(const std::string& symbol) {
    auto it = historical_volumes_.find(symbol);
    if (it == historical_volumes_.end() || it->second.empty()) {
        return 0;
    }
    
    const auto& volumes = it->second;
    uint64_t sum = std::accumulate(volumes.begin(), volumes.end(), 0ULL);
    return sum / volumes.size();
}

double PricingEngine::normalCDF(double x) {
    // Approximation of normal cumulative distribution function
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

double PricingEngine::normalPDF(double x) {
    // Normal probability density function
    return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
}

double PricingEngine::getTimeToExpiry(const std::chrono::system_clock::time_point& expiry) {
    auto now = std::chrono::system_clock::now();
    auto duration = expiry - now;
    auto days = std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24.0;
    return std::max(0.0, days / 365.0); // Convert to years
}

std::string PricingEngine::selectOptimalExchange(const MarketData& nse_data, const MarketData& bse_data) {
    // Prefer NSE for futures (more liquid as per requirements)
    // For cash, compare spreads and volumes
    
    double nse_spread = (nse_data.ask - nse_data.bid) / nse_data.price;
    double bse_spread = (bse_data.ask - bse_data.bid) / bse_data.price;
    
    // If spreads are similar, prefer higher volume
    if (std::abs(nse_spread - bse_spread) < 0.001) {
        return (nse_data.volume > bse_data.volume) ? "NSE" : "BSE";
    }
    
    // Otherwise prefer tighter spread
    return (nse_spread < bse_spread) ? "NSE" : "BSE";
}

// MarginCalculator implementation
MarginCalculator::MarginRequirement MarginCalculator::calculateMargin(
    const std::vector<OptionContract>& positions
) {
    MarginRequirement requirement;
    requirement.span_margin = 0.0;
    requirement.exposure_margin = 0.0;
    requirement.calculation_method = "SPAN";
    
    for (const auto& position : positions) {
        requirement.span_margin += calculateSpanMargin(position);
        requirement.exposure_margin += calculateExposureMargin(position);
    }
    
    requirement.total_margin = requirement.span_margin + requirement.exposure_margin;
    return requirement;
}

double MarginCalculator::calculateSpanMargin(const OptionContract& contract) {
    // Simplified SPAN margin calculation
    // In practice, this would be much more complex with risk scenarios
    
    double underlying_price = 100.0; // This should come from market data
    double margin_rate = 0.10; // 10% of underlying value as base margin
    
    if (contract.contract.instrument_type == "OPTION") {
        // For options, margin is typically the premium + percentage of underlying
        return contract.market_data.price + (underlying_price * contract.contract.lot_size * margin_rate);
    } else if (contract.contract.instrument_type == "FUTURE") {
        // For futures, margin is percentage of contract value
        return underlying_price * contract.contract.lot_size * margin_rate;
    }
    
    return 0.0;
}

double MarginCalculator::calculateExposureMargin(const OptionContract& contract) {
    // Exposure margin is typically a small percentage of contract value
    double underlying_price = 100.0; // This should come from market data
    double exposure_rate = 0.05; // 5% exposure margin
    
    return underlying_price * contract.contract.lot_size * exposure_rate;
}

} // namespace trading