#include "regime/RegimeDetector.h"
#include <numeric>
#include <algorithm>

namespace fluxback {

RegimeDetector::RegimeDetector(int lookback_window)
    : lookback_window(lookback_window), current_regime(Regime::SIDEWAYS) {
}

void RegimeDetector::reset() {
    prices.clear();
    volumes.clear();
    ranges.clear();
    current_regime = Regime::SIDEWAYS;
}

Regime RegimeDetector::update_and_get(const OHLCV& tick) {
    prices.push_back(tick.close);
    volumes.push_back(tick.volume);
    ranges.push_back(tick.high - tick.low);
    
    // Keep only lookback_window items
    if (prices.size() > static_cast<size_t>(lookback_window)) {
        prices.pop_front();
        volumes.pop_front();
        ranges.pop_front();
    }
    
    // Need enough data to classify
    if (prices.size() < static_cast<size_t>(lookback_window / 2)) {
        return Regime::SIDEWAYS;
    }
    
    // Calculate features
    double vol = calculate_realized_vol();
    double vol_zscore = calculate_volume_zscore();
    double avg_range = calculate_range_mean();
    
    // Classify
    current_regime = classify_regime(vol, vol_zscore, avg_range);
    
    return current_regime;
}

double RegimeDetector::calculate_realized_vol() {
    if (prices.size() < 2) return 0.0;
    
    std::vector<double> returns;
    for (size_t i = 1; i < prices.size(); ++i) {
        if (prices[i-1] > 0.0) {
            double ret = std::log(prices[i] / prices[i-1]);
            returns.push_back(ret);
        }
    }
    
    if (returns.empty()) return 0.0;
    
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    for (double ret : returns) {
        double diff = ret - mean;
        variance += diff * diff;
    }
    variance /= returns.size();
    
    return std::sqrt(variance);
}

double RegimeDetector::calculate_volume_zscore() {
    if (volumes.size() < 2) return 0.0;
    
    double mean = std::accumulate(volumes.begin(), volumes.end(), 0.0) / volumes.size();
    double variance = 0.0;
    for (double vol : volumes) {
        double diff = vol - mean;
        variance += diff * diff;
    }
    variance /= volumes.size();
    double stddev = std::sqrt(variance);
    
    if (stddev == 0.0) return 0.0;
    
    double current_volume = volumes.back();
    return (current_volume - mean) / stddev;
}

double RegimeDetector::calculate_range_mean() {
    if (ranges.empty()) return 0.0;
    return std::accumulate(ranges.begin(), ranges.end(), 0.0) / ranges.size();
}

Regime RegimeDetector::classify_regime(double vol, double vol_zscore, double avg_range) {
    // Simple threshold-based classification
    // TREND: high volatility, consistent direction (simplified: high vol, moderate range)
    // VOLATILE: very high volatility, high volume z-score, large ranges
    // SIDEWAYS: low volatility, low ranges
    
    // Normalize thresholds (these are heuristic and can be tuned)
    double vol_threshold_high = 0.02;  // 2% volatility
    double vol_threshold_low = 0.005;  // 0.5% volatility
    double vol_zscore_threshold = 1.5;
    double range_threshold_high = avg_range * 1.5;
    double range_threshold_low = avg_range * 0.5;
    
    // VOLATILE: very high volatility and volume spikes
    if (vol > vol_threshold_high && std::abs(vol_zscore) > vol_zscore_threshold) {
        return Regime::VOLATILE;
    }
    
    // TREND: moderate to high volatility, consistent ranges
    if (vol > vol_threshold_low && avg_range > range_threshold_low && avg_range < range_threshold_high) {
        return Regime::TREND;
    }
    
    // SIDEWAYS: low volatility, small ranges
    return Regime::SIDEWAYS;
}

} // namespace fluxback

