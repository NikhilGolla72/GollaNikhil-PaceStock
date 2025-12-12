#pragma once

#include "strategy/StrategyEngine.h"
#include "data/DataLoader.h"
#include "utils/ConfigParser.h"
#include <string>

namespace fluxback {

struct Fill {
    Order order;
    double fill_price;
    int filled_size;
    std::string timestamp;
    double slippage;
    
    Fill() : order(Order(Order::BUY, 0, 0.0, "")), fill_price(0.0), filled_size(0), timestamp(""), slippage(0.0) {}
    
    Fill(const Order& o, double fp, int fs, const std::string& ts, double sl)
        : order(o), fill_price(fp), filled_size(fs), timestamp(ts), slippage(sl) {}
};

struct Position {
    int size;  // positive = long, negative = short, zero = flat
    double avg_price;
    double unrealized_pnl;
    
    Position() : size(0), avg_price(0.0), unrealized_pnl(0.0) {}
};

class ExecutionSimulator {
public:
    explicit ExecutionSimulator(const StrategyConfig& cfg);
    
    // Execute an order and return fill
    Fill execute(const Order& order, const OHLCV& tick, double realized_volatility);
    
    // Get current position
    Position get_position() const { return position; }
    
    // Get current cash balance
    double get_cash() const { return cash; }
    
    // Reset simulator
    void reset(double initial_cash = 100000.0);

private:
    StrategyConfig config;
    Position position;
    double cash;
    double initial_cash;
    
    // Calculate slippage based on volatility and regime
    double calculate_slippage(const Order& order, double realized_volatility, double current_price);
    
    // Update position after fill
    void update_position(const Fill& fill);
};

} // namespace fluxback

