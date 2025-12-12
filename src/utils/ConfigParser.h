#pragma once

#include <string>
#include <map>

namespace fluxback {

struct StrategyConfig {
    std::string name;
    std::string type;
    std::string symbol;
    std::string timeframe;
    
    // Entry parameters
    int fast_sma = 10;
    int slow_sma = 20;
    bool use_rsi_filter = false;
    double rsi_overbought = 70.0;
    double rsi_oversold = 30.0;
    bool use_vol_filter = false;
    double vol_threshold = 0.05; // annualized realized vol threshold
    
    // Exit parameters
    double stop_loss_pct = 0.5;
    double take_profit_pct = 1.0;
    
    // Risk parameters
    int position_size = 100;
    
    // Execution parameters
    struct SlippageConfig {
        std::string type = "fixed"; // "fixed" or "adaptive"
        int base_ticks = 1;
        double vol_multiplier = 0.001;
        double vol_low = 0.01;
        double vol_high = 0.05;
        double low_factor = 0.5;
        double high_factor = 1.5;
    } slippage;

    // Regime handling
    bool exclude_volatile_regime = false;
};

class ConfigParser {
public:
    static StrategyConfig parse_yaml(const std::string& yaml_path);
    static StrategyConfig parse_json(const std::string& json_path);
    
private:
    static StrategyConfig parse_yaml_simple(const std::string& yaml_path);
    static std::string trim(const std::string& str);
    static std::string get_value(const std::string& line, const std::string& key);
    static int get_int_value(const std::string& line, const std::string& key, int default_val = 0);
    static double get_double_value(const std::string& line, const std::string& key, double default_val = 0.0);
    static std::string get_string_value(const std::string& line, const std::string& key, const std::string& default_val = "");
};

} // namespace fluxback

