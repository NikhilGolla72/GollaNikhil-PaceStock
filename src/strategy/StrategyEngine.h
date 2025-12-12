#pragma once

#include "utils/ConfigParser.h"
#include "data/DataLoader.h"
#include "indicators/IndicatorEngine.h"
#include <vector>

namespace fluxback {

struct Order {
    enum Type { BUY, SELL };
    Type type;
    int size;
    double price;
    std::string timestamp;
    
    Order(Type t, int s, double p, const std::string& ts)
        : type(t), size(s), price(p), timestamp(ts) {}
};

class StrategyEngine {
public:
    explicit StrategyEngine(const StrategyConfig& cfg);
    
    // Evaluate strategy on new tick and return orders
    std::vector<Order> on_tick(const OHLCV& tick, IndicatorEngine& ie);
    
    // Get current position state
    bool is_long() const { return current_position > 0; }
    bool is_short() const { return current_position < 0; }
    bool is_flat() const { return current_position == 0; }
    int get_position() const { return current_position; }
    
    // Reset strategy state
    void reset();

private:
    StrategyConfig config;
    int current_position;
    double entry_price;
    bool position_opened;
    
    // Track previous SMA values for crossover detection
    double prev_fast_sma;
    double prev_slow_sma;
    bool sma_initialized;
    
    // Check entry conditions
    bool check_entry_long(const OHLCV& tick, IndicatorEngine& ie);
    bool check_entry_short(const OHLCV& tick, IndicatorEngine& ie);
    
    // Check exit conditions
    bool check_stop_loss(const OHLCV& tick);
    bool check_take_profit(const OHLCV& tick);
    bool check_exit_signal(const OHLCV& tick, IndicatorEngine& ie);
};

} // namespace fluxback

