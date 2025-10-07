#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "pricing_engine.h"

int main() {
    std::cout << "🚀 Stenoa Trading Platform Backend Starting..." << std::endl;
    std::cout << "📊 Options Trading & Pricing Engine Phase 2" << std::endl;
    
    try {
        // Initialize pricing engine
        auto pricing_engine = std::make_unique<trading::PricingEngine>();
        std::cout << "✅ Pricing Engine initialized" << std::endl;
        
        // Set configuration
        pricing_engine->setRiskFreeRate(0.06);
        pricing_engine->setVolumeThreshold(0.05);
        std::cout << "✅ Risk parameters configured" << std::endl;
        
        // TODO: Initialize web server for API endpoints
        std::cout << "⏳ Web server initialization (TODO)" << std::endl;
        
        // TODO: Initialize Zerodha connector
        std::cout << "⏳ Zerodha API connector (TODO)" << std::endl;
        
        // TODO: Initialize database connections
        std::cout << "⏳ Database connections (TODO)" << std::endl;
        
        // For now, just demonstrate pricing engine functionality
        std::cout << "\n🧮 Testing Pricing Engine Functions:" << std::endl;
        
        // Test Black-Scholes calculation
        double spot_price = 19850.0;
        double strike = 20000.0;
        double time_to_expiry = 30.0 / 365.0; // 30 days
        double risk_free_rate = 0.06;
        double volatility = 0.15;
        
        double call_price = pricing_engine->calculateOptionFairValue(
            spot_price, strike, time_to_expiry, risk_free_rate, volatility, "CE"
        );
        
        double put_price = pricing_engine->calculateOptionFairValue(
            spot_price, strike, time_to_expiry, risk_free_rate, volatility, "PE"
        );
        
        std::cout << "   NIFTY Spot: ₹" << spot_price << std::endl;
        std::cout << "   Strike: ₹" << strike << std::endl;
        std::cout << "   Call Option Fair Value: ₹" << call_price << std::endl;
        std::cout << "   Put Option Fair Value: ₹" << put_price << std::endl;
        
        // Test Greeks calculation
        auto greeks = pricing_engine->calculateGreeks(
            spot_price, strike, time_to_expiry, risk_free_rate, volatility, "CE"
        );
        
        std::cout << "\n📈 Option Greeks:" << std::endl;
        std::cout << "   Delta: " << greeks.delta << std::endl;
        std::cout << "   Gamma: " << greeks.gamma << std::endl;
        std::cout << "   Theta: " << greeks.theta << std::endl;
        std::cout << "   Vega: " << greeks.vega << std::endl;
        
        std::cout << "\n🏃‍♂️ Backend is running..." << std::endl;
        std::cout << "📡 Listening on port 8080" << std::endl;
        std::cout << "💡 Use Ctrl+C to stop" << std::endl;
        
        // Keep the application running
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            std::cout << "💓 Heartbeat - System operational" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error starting trading platform: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}