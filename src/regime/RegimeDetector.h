#pragma once

#include "data/DataLoader.h"
#include <deque>
#include <vector>
#include <cmath>

namespace fluxback {

enum class Regime {
    TREND,
    VOLATILE,
    SIDEWAYS
};

class RegimeDetector {
public:
    RegimeDetector(int lookback_window = 20);
    
    // Update with new tick and return current regime
    Regime update_and_get(const OHLCV& tick);
    
    // Get current regime without updating
    Regime get_current_regime() const { return current_regime; }
    
    // Reset detector
    void reset();

private:
    int lookback_window;
    Regime current_regime;
    
    std::deque<double> prices;
    std::deque<double> volumes;
    std::deque<double> ranges; // high - low
    
    // Features for classification
    double calculate_realized_vol();
    double calculate_volume_zscore();
    double calculate_range_mean();
    
    // Simple threshold-based classification
    Regime classify_regime(double vol, double vol_zscore, double avg_range);
};

} // namespace fluxback

