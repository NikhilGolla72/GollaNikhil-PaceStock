#include "strategy/StrategyEngine.h"
#include <algorithm>

namespace fluxback {

StrategyEngine::StrategyEngine(const StrategyConfig& cfg)
    : config(cfg), current_position(0), entry_price(0.0), position_opened(false),
      prev_fast_sma(0.0), prev_slow_sma(0.0), sma_initialized(false) {
}

void StrategyEngine::reset() {
    current_position = 0;
    entry_price = 0.0;
    position_opened = false;
    prev_fast_sma = 0.0;
    prev_slow_sma = 0.0;
    sma_initialized = false;
}

std::vector<Order> StrategyEngine::on_tick(const OHLCV& tick, IndicatorEngine& ie) {
    std::vector<Order> orders;
    
    // Update SMAs for crossover detection
    double fast_sma = ie.get_sma(config.fast_sma);
    double slow_sma = ie.get_sma(config.slow_sma);
    double realized_vol = ie.get_realized_vol(20);

    // Volatility filter: skip trading when realized vol above threshold
    if (config.use_vol_filter && realized_vol > config.vol_threshold) {
        // Still update previous SMA state to avoid stale crossover detection
        if (fast_sma > 0.0 && slow_sma > 0.0) {
            prev_fast_sma = fast_sma;
            prev_slow_sma = slow_sma;
            sma_initialized = true;
        }
        return orders;
    }
    
    // Check exit conditions first (stop loss, take profit, signal reversal)
    if (current_position != 0) {
        if (check_stop_loss(tick) || check_take_profit(tick) || check_exit_signal(tick, ie)) {
            // Close position
            Order exit_order(current_position > 0 ? Order::SELL : Order::BUY,
                           std::abs(current_position),
                           tick.close,
                           tick.timestamp);
            orders.push_back(exit_order);
            current_position = 0;
            entry_price = 0.0;
            position_opened = false;
            return orders;
        }
    }
    
    // Check entry conditions only if flat
    if (current_position == 0 && sma_initialized) {
        if (check_entry_long(tick, ie)) {
            Order buy_order(Order::BUY, config.position_size, tick.close, tick.timestamp);
            orders.push_back(buy_order);
            current_position = config.position_size;
            entry_price = tick.close;
            position_opened = true;
        } else if (check_entry_short(tick, ie)) {
            Order sell_order(Order::SELL, config.position_size, tick.close, tick.timestamp);
            orders.push_back(sell_order);
            current_position = -config.position_size;
            entry_price = tick.close;
            position_opened = true;
        }
    }
    
    // Update previous SMA values
    if (fast_sma > 0.0 && slow_sma > 0.0) {
        prev_fast_sma = fast_sma;
        prev_slow_sma = slow_sma;
        sma_initialized = true;
    }
    
    return orders;
}

bool StrategyEngine::check_entry_long(const OHLCV& tick, IndicatorEngine& ie) {
    // SMA crossover: fast crosses above slow
    double fast_sma = ie.get_sma(config.fast_sma);
    double slow_sma = ie.get_sma(config.slow_sma);
    
    if (fast_sma <= 0.0 || slow_sma <= 0.0) return false;
    if (prev_fast_sma <= 0.0 || prev_slow_sma <= 0.0) return false;
    
    // Crossover: fast was below slow, now fast is above slow
    bool crossover = (prev_fast_sma <= prev_slow_sma) && (fast_sma > slow_sma);
    
    if (!crossover) return false;
    
    // Optional RSI filter
    if (config.use_rsi_filter) {
        double rsi = ie.get_rsi(14);
        if (rsi > config.rsi_overbought) return false; // avoid overbought
        if (rsi < config.rsi_oversold) {
            // Oversold is ok for longs; pass through
        }
    }
    
    return true;
}

bool StrategyEngine::check_entry_short(const OHLCV& tick, IndicatorEngine& ie) {
    // SMA crossover: fast crosses below slow
    double fast_sma = ie.get_sma(config.fast_sma);
    double slow_sma = ie.get_sma(config.slow_sma);
    
    if (fast_sma <= 0.0 || slow_sma <= 0.0) return false;
    if (prev_fast_sma <= 0.0 || prev_slow_sma <= 0.0) return false;
    
    // Crossover: fast was above slow, now fast is below slow
    bool crossover = (prev_fast_sma >= prev_slow_sma) && (fast_sma < slow_sma);
    
    if (!crossover) return false;
    
    // Optional RSI filter
    if (config.use_rsi_filter) {
        double rsi = ie.get_rsi(14);
        if (rsi < config.rsi_oversold) return false; // avoid oversold for shorts
        if (rsi > config.rsi_overbought) {
            // Overbought is ok for shorts; pass through
        }
    }
    
    return true;
}

bool StrategyEngine::check_stop_loss(const OHLCV& tick) {
    if (entry_price <= 0.0) return false;
    
    if (current_position > 0) {
        // Long position: stop loss if price drops below entry - stop_loss_pct
        double stop_price = entry_price * (1.0 - config.stop_loss_pct / 100.0);
        return tick.low <= stop_price;
    } else if (current_position < 0) {
        // Short position: stop loss if price rises above entry + stop_loss_pct
        double stop_price = entry_price * (1.0 + config.stop_loss_pct / 100.0);
        return tick.high >= stop_price;
    }
    
    return false;
}

bool StrategyEngine::check_take_profit(const OHLCV& tick) {
    if (entry_price <= 0.0) return false;
    
    if (current_position > 0) {
        // Long position: take profit if price rises above entry + take_profit_pct
        double tp_price = entry_price * (1.0 + config.take_profit_pct / 100.0);
        return tick.high >= tp_price;
    } else if (current_position < 0) {
        // Short position: take profit if price drops below entry - take_profit_pct
        double tp_price = entry_price * (1.0 - config.take_profit_pct / 100.0);
        return tick.low <= tp_price;
    }
    
    return false;
}

bool StrategyEngine::check_exit_signal(const OHLCV& tick, IndicatorEngine& ie) {
    // Exit on reverse crossover
    double fast_sma = ie.get_sma(config.fast_sma);
    double slow_sma = ie.get_sma(config.slow_sma);
    
    if (fast_sma <= 0.0 || slow_sma <= 0.0) return false;
    if (prev_fast_sma <= 0.0 || prev_slow_sma <= 0.0) return false;
    
    if (current_position > 0) {
        // Long position: exit if fast crosses below slow
        return (prev_fast_sma >= prev_slow_sma) && (fast_sma < slow_sma);
    } else if (current_position < 0) {
        // Short position: exit if fast crosses above slow
        return (prev_fast_sma <= prev_slow_sma) && (fast_sma > slow_sma);
    }
    
    return false;
}

} // namespace fluxback

