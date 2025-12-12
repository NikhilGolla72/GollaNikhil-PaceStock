#include "indicators/IndicatorEngine.h"
#include <algorithm>
#include <numeric>

namespace fluxback {

IndicatorEngine::IndicatorEngine()
    : latest_price(0.0), latest_volume(0), rsi_avg_gain(0.0), rsi_avg_loss(0.0),
      rsi_initialized(false), rsi_window(14) {
}

void IndicatorEngine::add_price(double price, long volume) {
    latest_price = price;
    latest_volume = volume;
    
    // Update all indicators
    update_realized_vol(price);
    update_rsi(price);
    update_vwap(price, volume, 20);
}

void IndicatorEngine::reset() {
    sma_queues.clear();
    sma_sums.clear();
    ema_values.clear();
    ema_initialized.clear();
    price_changes.clear();
    returns.clear();
    price_volume_pairs.clear();
    vwap_sums_price_volume.clear();
    vwap_sums_volume.clear();
    rsi_initialized = false;
    rsi_avg_gain = 0.0;
    rsi_avg_loss = 0.0;
    latest_price = 0.0;
    latest_volume = 0;
}

double IndicatorEngine::get_sma(int window) {
    if (window <= 0) return 0.0;
    return update_sma(latest_price, window);
}

double IndicatorEngine::update_sma(double price, int window) {
    auto& queue = sma_queues[window];
    auto& sum = sma_sums[window];
    
    queue.push_back(price);
    sum += price;
    
    if (queue.size() > static_cast<size_t>(window)) {
        sum -= queue.front();
        queue.pop_front();
    }
    
    if (queue.empty()) return 0.0;
    return sum / queue.size();
}

double IndicatorEngine::get_ema(int window) {
    if (window <= 0) return 0.0;
    return update_ema(latest_price, window);
}

double IndicatorEngine::update_ema(double price, int window) {
    auto& ema = ema_values[window];
    bool& initialized = ema_initialized[window];
    
    if (!initialized) {
        ema = price;
        initialized = true;
    } else {
        double alpha = 2.0 / (window + 1.0);
        ema = alpha * price + (1.0 - alpha) * ema;
    }
    
    return ema;
}

void IndicatorEngine::update_rsi(double price) {
    if (price_changes.empty()) {
        price_changes.push_back(0.0);
        return;
    }
    
    double change = price - latest_price;
    price_changes.push_back(change);
    
    if (price_changes.size() > static_cast<size_t>(rsi_window + 1)) {
        price_changes.pop_front();
    }
    
    if (!rsi_initialized && price_changes.size() >= static_cast<size_t>(rsi_window + 1)) {
        // Initialize RSI with average gain/loss
        double sum_gains = 0.0;
        double sum_losses = 0.0;
        
        for (size_t i = 1; i < price_changes.size(); ++i) {
            if (price_changes[i] > 0) {
                sum_gains += price_changes[i];
            } else {
                sum_losses += std::abs(price_changes[i]);
            }
        }
        
        rsi_avg_gain = sum_gains / rsi_window;
        rsi_avg_loss = sum_losses / rsi_window;
        rsi_initialized = true;
    } else if (rsi_initialized && price_changes.size() >= 2) {
        // Welles Wilder smoothing
        double change = price_changes.back();
        double gain = change > 0 ? change : 0.0;
        double loss = change < 0 ? std::abs(change) : 0.0;
        
        rsi_avg_gain = (rsi_avg_gain * (rsi_window - 1) + gain) / rsi_window;
        rsi_avg_loss = (rsi_avg_loss * (rsi_window - 1) + loss) / rsi_window;
    }
}

double IndicatorEngine::get_rsi(int window) {
    if (window != rsi_window) {
        // Reset and recalculate with new window
        rsi_window = window;
        rsi_initialized = false;
        price_changes.clear();
        rsi_avg_gain = 0.0;
        rsi_avg_loss = 0.0;
        return 0.0;
    }
    
    if (!rsi_initialized || rsi_avg_loss == 0.0) {
        return 50.0; // Neutral RSI
    }
    
    double rs = rsi_avg_gain / rsi_avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

void IndicatorEngine::update_realized_vol(double price) {
    if (returns.empty()) {
        returns.push_back(0.0);
        return;
    }
    
    double prev_price = latest_price;
    if (prev_price > 0.0) {
        double ret = std::log(price / prev_price);
        returns.push_back(ret);
    } else {
        returns.push_back(0.0);
    }
    
    // Keep only last 20 returns for volatility calculation
    if (returns.size() > 20) {
        returns.pop_front();
    }
}

double IndicatorEngine::get_realized_vol(int window) {
    if (returns.size() < 2) return 0.0;
    
    size_t calc_window = std::min(returns.size(), static_cast<size_t>(window));
    if (calc_window < 2) return 0.0;
    
    // Calculate mean return
    double mean = 0.0;
    size_t start_idx = returns.size() - calc_window;
    for (size_t i = start_idx; i < returns.size(); ++i) {
        mean += returns[i];
    }
    mean /= calc_window;
    
    // Calculate variance
    double variance = 0.0;
    for (size_t i = start_idx; i < returns.size(); ++i) {
        double diff = returns[i] - mean;
        variance += diff * diff;
    }
    variance /= (calc_window - 1);
    
    // Annualized volatility (assuming 252 trading days, ~390 minutes per day)
    double daily_vol = std::sqrt(variance * 390);
    return daily_vol * std::sqrt(252.0);
}

void IndicatorEngine::update_vwap(double price, long volume, int window) {
    price_volume_pairs.push_back({price, volume});
    
    auto& sum_pv = vwap_sums_price_volume[window];
    auto& sum_v = vwap_sums_volume[window];
    
    sum_pv += price * volume;
    sum_v += volume;
    
    if (price_volume_pairs.size() > static_cast<size_t>(window)) {
        auto& old_pair = price_volume_pairs.front();
        sum_pv -= old_pair.first * old_pair.second;
        sum_v -= old_pair.second;
        price_volume_pairs.pop_front();
    }
}

double IndicatorEngine::get_vwap(int window) {
    auto it_pv = vwap_sums_price_volume.find(window);
    auto it_v = vwap_sums_volume.find(window);
    
    if (it_pv == vwap_sums_price_volume.end() || it_v == vwap_sums_volume.end()) {
        return 0.0;
    }
    
    if (it_v->second == 0) return 0.0;
    return it_pv->second / it_v->second;
}

} // namespace fluxback

