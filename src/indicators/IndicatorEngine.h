#pragma once

#include <deque>
#include <unordered_map>
#include <cmath>

namespace fluxback {

class IndicatorEngine {
public:
    IndicatorEngine();
    
    // Update with new price/volume
    void add_price(double price, long volume = 0);
    
    // Simple Moving Average
    double get_sma(int window);
    
    // Exponential Moving Average
    double get_ema(int window);
    
    // Relative Strength Index
    double get_rsi(int window = 14);
    
    // Realized Volatility (standard deviation of returns)
    double get_realized_vol(int window = 20);
    
    // Volume Weighted Average Price
    double get_vwap(int window = 20);
    
    // Get latest price
    double get_latest_price() const { return latest_price; }
    
    // Reset all indicators
    void reset();

private:
    double latest_price;
    long latest_volume;
    
    // SMA storage: window -> (queue, sum)
    std::unordered_map<int, std::deque<double>> sma_queues;
    std::unordered_map<int, double> sma_sums;
    
    // EMA storage: window -> current EMA value
    std::unordered_map<int, double> ema_values;
    std::unordered_map<int, bool> ema_initialized;
    
    // RSI storage
    std::deque<double> price_changes;
    double rsi_avg_gain;
    double rsi_avg_loss;
    bool rsi_initialized;
    int rsi_window;
    
    // Realized volatility storage
    std::deque<double> returns;
    
    // VWAP storage
    std::deque<std::pair<double, long>> price_volume_pairs; // (price, volume)
    std::unordered_map<int, double> vwap_sums_price_volume;
    std::unordered_map<int, long> vwap_sums_volume;
    
    // Helper methods
    double update_sma(double price, int window);
    double update_ema(double price, int window);
    void update_rsi(double price);
    void update_realized_vol(double price);
    void update_vwap(double price, long volume, int window);
};

} // namespace fluxback

