#include "utils/ConfigParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace fluxback {

std::string ConfigParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::string ConfigParser::get_value(const std::string& line, const std::string& key) {
    size_t pos = line.find(key + ":");
    if (pos == std::string::npos) return "";
    
    size_t start = pos + key.length() + 1;
    std::string value = line.substr(start);
    value = trim(value);
    
    // Remove quotes if present
    if (value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.length() - 2);
    }
    
    return value;
}

int ConfigParser::get_int_value(const std::string& line, const std::string& key, int default_val) {
    std::string val = get_value(line, key);
    if (val.empty()) return default_val;
    try {
        return std::stoi(val);
    } catch (...) {
        return default_val;
    }
}

double ConfigParser::get_double_value(const std::string& line, const std::string& key, double default_val) {
    std::string val = get_value(line, key);
    if (val.empty()) return default_val;
    try {
        return std::stod(val);
    } catch (...) {
        return default_val;
    }
}

std::string ConfigParser::get_string_value(const std::string& line, const std::string& key, const std::string& default_val) {
    std::string val = get_value(line, key);
    return val.empty() ? default_val : val;
}

StrategyConfig ConfigParser::parse_yaml_simple(const std::string& yaml_path) {
    StrategyConfig config;
    std::ifstream file(yaml_path);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << yaml_path << std::endl;
        return config;
    }
    
    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        
        // Detect sections
        if (line.find("strategy:") != std::string::npos) {
            current_section = "strategy";
            continue;
        } else if (line.find("risk:") != std::string::npos) {
            current_section = "risk";
            continue;
        } else if (line.find("execution:") != std::string::npos) {
            current_section = "execution";
            continue;
        } else if (line.find("entry:") != std::string::npos) {
            current_section = "entry";
            continue;
        } else if (line.find("exit:") != std::string::npos) {
            current_section = "exit";
            continue;
        }
        
        // Parse fields
        if (current_section == "strategy") {
            if (line.find("name:") != std::string::npos) {
                config.name = get_string_value(line, "name");
            } else if (line.find("type:") != std::string::npos) {
                config.type = get_string_value(line, "type");
            } else if (line.find("symbol:") != std::string::npos) {
                config.symbol = get_string_value(line, "symbol");
            } else if (line.find("timeframe:") != std::string::npos) {
                config.timeframe = get_string_value(line, "timeframe");
            }
        } else if (current_section == "entry") {
            if (line.find("fast:") != std::string::npos) {
                config.fast_sma = get_int_value(line, "fast", 10);
            } else if (line.find("slow:") != std::string::npos) {
                config.slow_sma = get_int_value(line, "slow", 20);
            } else if (line.find("rsi_overbought:") != std::string::npos) {
                config.rsi_overbought = get_double_value(line, "rsi_overbought", 70.0);
                config.use_rsi_filter = true;
            } else if (line.find("rsi_oversold:") != std::string::npos) {
                config.rsi_oversold = get_double_value(line, "rsi_oversold", 30.0);
                config.use_rsi_filter = true;
            } else if (line.find("vol_threshold:") != std::string::npos) {
                config.vol_threshold = get_double_value(line, "vol_threshold", 0.05);
                config.use_vol_filter = true;
            }
        } else if (current_section == "exit") {
            if (line.find("stop_loss_pct:") != std::string::npos) {
                config.stop_loss_pct = get_double_value(line, "stop_loss_pct", 0.5);
            } else if (line.find("take_profit_pct:") != std::string::npos) {
                config.take_profit_pct = get_double_value(line, "take_profit_pct", 1.0);
            }
        } else if (current_section == "risk") {
            if (line.find("position_size:") != std::string::npos) {
                config.position_size = get_int_value(line, "position_size", 100);
            }
        } else if (current_section == "execution") {
            if (line.find("type:") != std::string::npos) {
                config.slippage.type = get_string_value(line, "type", "fixed");
            } else if (line.find("base_ticks:") != std::string::npos) {
                config.slippage.base_ticks = get_int_value(line, "base_ticks", 1);
            } else if (line.find("vol_multiplier:") != std::string::npos) {
                config.slippage.vol_multiplier = get_double_value(line, "vol_multiplier", 0.001);
            } else if (line.find("vol_low:") != std::string::npos) {
                config.slippage.vol_low = get_double_value(line, "vol_low", 0.01);
            } else if (line.find("vol_high:") != std::string::npos) {
                config.slippage.vol_high = get_double_value(line, "vol_high", 0.05);
            } else if (line.find("low_factor:") != std::string::npos) {
                config.slippage.low_factor = get_double_value(line, "low_factor", 0.5);
            } else if (line.find("high_factor:") != std::string::npos) {
                config.slippage.high_factor = get_double_value(line, "high_factor", 1.5);
            }
        } else if (current_section == "strategy") {
            if (line.find("exclude_volatile_regime:") != std::string::npos) {
                std::string val = get_string_value(line, "exclude_volatile_regime", "false");
                config.exclude_volatile_regime = (val == "true" || val == "1");
            }
        }
    }
    
    file.close();
    return config;
}

StrategyConfig ConfigParser::parse_yaml(const std::string& yaml_path) {
    return parse_yaml_simple(yaml_path);
}

StrategyConfig ConfigParser::parse_json(const std::string& json_path) {
    // For MVP, JSON parsing can be added later
    // For now, redirect to YAML parser
    return parse_yaml(json_path);
}

} // namespace fluxback

