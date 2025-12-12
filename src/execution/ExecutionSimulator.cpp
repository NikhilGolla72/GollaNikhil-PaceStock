#include "execution/ExecutionSimulator.h"
#include <cmath>
#include <algorithm>

namespace fluxback {

ExecutionSimulator::ExecutionSimulator(const StrategyConfig& cfg)
    : config(cfg), cash(100000.0), initial_cash(100000.0) {
    position = Position();
}

void ExecutionSimulator::reset(double initial_cash) {
    this->initial_cash = initial_cash;
    this->cash = initial_cash;
    position = Position();
}

Fill ExecutionSimulator::execute(const Order& order, const OHLCV& tick, double realized_volatility) {
    double current_price = tick.close;
    
    // Calculate slippage
    double slippage = calculate_slippage(order, realized_volatility, current_price);
    
    // Determine fill price (apply slippage in adverse direction)
    double fill_price = current_price;
    if (order.type == Order::BUY) {
        fill_price = current_price + slippage; // Buy at higher price (adverse)
    } else {
        fill_price = current_price - slippage; // Sell at lower price (adverse)
    }
    
    // Ensure fill price is within tick's high/low range
    fill_price = std::max(tick.low, std::min(tick.high, fill_price));
    
    // Calculate cost/proceeds
    double cost = fill_price * order.size;
    
    // Update cash
    if (order.type == Order::BUY) {
        cash -= cost;
    } else {
        cash += cost;
    }
    
    // Create fill
    Fill fill(order, fill_price, order.size, tick.timestamp, slippage);
    
    // Update position
    update_position(fill);
    
    return fill;
}

double ExecutionSimulator::calculate_slippage(const Order& order, double realized_volatility, double current_price) {
    double slippage = 0.0;
    
    if (config.slippage.type == "adaptive") {
        // Adaptive slippage: base_ticks * tick_size + vol_multiplier * realized_volatility
        // For simplicity, assume tick_size = 0.01 (1 cent for most stocks)
        double tick_size = 0.01;
        double base_slippage = config.slippage.base_ticks * tick_size;
        double vol_component = config.slippage.vol_multiplier * realized_volatility * current_price;

        // Dynamic scaling based on realized volatility bands
        double factor = 1.0;
        if (realized_volatility < config.slippage.vol_low) {
            factor = config.slippage.low_factor;
        } else if (realized_volatility > config.slippage.vol_high) {
            factor = config.slippage.high_factor;
        }

        slippage = (base_slippage + vol_component) * factor;
    } else {
        // Fixed slippage
        double tick_size = 0.01;
        slippage = config.slippage.base_ticks * tick_size;
    }
    
    return slippage;
}

void ExecutionSimulator::update_position(const Fill& fill) {
    if (fill.order.type == Order::BUY) {
        if (position.size < 0) {
            // Closing short position
            int close_size = std::min(std::abs(position.size), fill.filled_size);
            double pnl = (position.avg_price - fill.fill_price) * close_size;
            position.size += close_size;
            
            // If we're opening a new long position
            if (fill.filled_size > close_size) {
                int new_size = fill.filled_size - close_size;
                position.avg_price = fill.fill_price;
                position.size = new_size;
            } else {
                position.size = 0;
                position.avg_price = 0.0;
            }
        } else {
            // Opening or adding to long position
            if (position.size == 0) {
                position.avg_price = fill.fill_price;
                position.size = fill.filled_size;
            } else {
                // Average price calculation
                double total_cost = position.avg_price * position.size + fill.fill_price * fill.filled_size;
                position.size += fill.filled_size;
                position.avg_price = total_cost / position.size;
            }
        }
    } else { // SELL
        if (position.size > 0) {
            // Closing long position
            int close_size = std::min(position.size, fill.filled_size);
            double pnl = (fill.fill_price - position.avg_price) * close_size;
            position.size -= close_size;
            
            // If we're opening a new short position
            if (fill.filled_size > close_size) {
                int new_size = fill.filled_size - close_size;
                position.avg_price = fill.fill_price;
                position.size = -new_size;
            } else {
                position.size = 0;
                position.avg_price = 0.0;
            }
        } else {
            // Opening or adding to short position
            if (position.size == 0) {
                position.avg_price = fill.fill_price;
                position.size = -fill.filled_size;
            } else {
                // Average price calculation for shorts
                double total_proceeds = std::abs(position.avg_price * position.size) + fill.fill_price * fill.filled_size;
                position.size -= fill.filled_size;
                position.avg_price = total_proceeds / std::abs(position.size);
            }
        }
    }
}

} // namespace fluxback

