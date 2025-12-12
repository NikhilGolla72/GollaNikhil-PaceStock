#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "indicators/IndicatorEngine.h"
#include <cmath>
#include <vector>

using namespace fluxback;

TEST_CASE("SMA Calculation", "[indicators]") {
    IndicatorEngine ie;
    
    // Test SMA with window 5
    std::vector<double> prices = {100.0, 101.0, 102.0, 103.0, 104.0, 105.0};
    std::vector<double> expected_sma = {0.0, 0.0, 0.0, 0.0, 0.0, 102.0}; // First 5 values average
    
    for (size_t i = 0; i < prices.size(); ++i) {
        ie.add_price(prices[i]);
        double sma = ie.get_sma(5);
        
        if (i >= 4) { // After 5 values
            double expected = (prices[i-4] + prices[i-3] + prices[i-2] + prices[i-1] + prices[i]) / 5.0;
            REQUIRE(std::abs(sma - expected) < 0.01);
        }
    }
}

TEST_CASE("EMA Calculation", "[indicators]") {
    IndicatorEngine ie;
    
    // Test EMA with window 3
    std::vector<double> prices = {100.0, 101.0, 102.0, 103.0, 104.0};
    
    for (double price : prices) {
        ie.add_price(price);
        double ema = ie.get_ema(3);
        REQUIRE(ema > 0.0);
    }
    
    // EMA should be between min and max prices
    double final_ema = ie.get_ema(3);
    REQUIRE(final_ema >= 100.0);
    REQUIRE(final_ema <= 104.0);
}

TEST_CASE("RSI Calculation", "[indicators]") {
    IndicatorEngine ie;
    
    // Create a sequence that should produce RSI values
    std::vector<double> prices;
    for (int i = 0; i < 20; ++i) {
        prices.push_back(100.0 + i * 0.5); // Upward trend
    }
    
    for (double price : prices) {
        ie.add_price(price);
    }
    
    double rsi = ie.get_rsi(14);
    REQUIRE(rsi >= 0.0);
    REQUIRE(rsi <= 100.0);
    
    // Upward trend should produce RSI > 50
    REQUIRE(rsi > 50.0);
}

TEST_CASE("Realized Volatility", "[indicators]") {
    IndicatorEngine ie;
    
    // Create price sequence
    std::vector<double> prices = {100.0, 101.0, 100.5, 101.5, 100.8, 101.2};
    
    for (double price : prices) {
        ie.add_price(price);
    }
    
    double vol = ie.get_realized_vol(20);
    REQUIRE(vol >= 0.0);
}

TEST_CASE("Indicator Reset", "[indicators]") {
    IndicatorEngine ie;
    
    // Add some data
    for (int i = 0; i < 10; ++i) {
        ie.add_price(100.0 + i);
    }
    
    // Reset
    ie.reset();
    
    // After reset, indicators should be zero/uninitialized
    double sma = ie.get_sma(5);
    REQUIRE(sma == 0.0);
}

