#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include "data/DataLoader.h"
#include "indicators/IndicatorEngine.h"
#include "strategy/StrategyEngine.h"
#include "execution/ExecutionSimulator.h"
#include "analytics/Analytics.h"
#include "regime/RegimeDetector.h"
#include "utils/ConfigParser.h"

using namespace fluxback;

void print_usage() {
    std::cout << "FluxBack - Regime-Aware C++ Backtesting Engine\n\n";
    std::cout << "Usage:\n";
    std::cout << "  fluxback run --strategy <yaml> --data <csv> [--out <json>]\n";
    std::cout << "  fluxback benchmark --strategy <yaml> --data <csv> [--parallel <n>]\n";
    std::cout << "  fluxback stats --results <json>\n\n";
    std::cout << "Examples:\n";
    std::cout << "  fluxback run --strategy config/sma_demo.yaml --data demo/RELIANCE_1m.csv --out results/sma_demo.json\n";
    std::cout << "  fluxback stats --results results/sma_demo.json\n";
}

void print_summary(const BacktestSummary& summary) {
    std::cout << "\n=== Backtest Summary ===\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Initial Cash:     $" << summary.initial_cash << "\n";
    std::cout << "Final Cash:       $" << summary.final_cash << "\n";
    std::cout << "Total Return:     " << summary.total_return_pct << "%\n";
    std::cout << "Annualized Return: " << summary.annualized_return_pct << "%\n";
    std::cout << "Sharpe Ratio:     " << std::setprecision(4) << summary.sharpe_ratio << "\n";
    std::cout << "Max Drawdown:     " << std::setprecision(2) << summary.max_drawdown_pct << "%\n";
    std::cout << "\n=== Trade Statistics ===\n";
    std::cout << "Total Trades:     " << summary.total_trades << "\n";
    std::cout << "Winning Trades:   " << summary.winning_trades << "\n";
    std::cout << "Losing Trades:    " << summary.losing_trades << "\n";
    std::cout << "Win Rate:         " << summary.win_rate_pct << "%\n";
    std::cout << "Avg Win:          $" << summary.avg_win_pct << "\n";
    std::cout << "Avg Loss:         $" << summary.avg_loss_pct << "\n";
    std::cout << "Profit Factor:    " << std::setprecision(4) << summary.profit_factor << "\n";
    
    if (!summary.trades_by_regime.empty()) {
        std::cout << "\n=== Per-Regime Statistics ===\n";
        for (const auto& [regime, count] : summary.trades_by_regime) {
            std::string regime_str;
            switch (regime) {
                case Regime::TREND: regime_str = "TREND"; break;
                case Regime::VOLATILE: regime_str = "VOLATILE"; break;
                case Regime::SIDEWAYS: regime_str = "SIDEWAYS"; break;
            }
            std::cout << regime_str << ": " << count << " trades, PnL: $" 
                      << std::setprecision(2) << summary.pnl_by_regime.at(regime) << "\n";
        }
    }
}

int run_backtest(const std::string& strategy_path, const std::string& data_path, const std::string& output_path) {
    // Load configuration
    StrategyConfig config = ConfigParser::parse_yaml(strategy_path);
    if (config.name.empty()) {
        std::cerr << "Error: Failed to parse strategy configuration.\n";
        return 1;
    }
    
    // Initialize components
    DataLoader loader(data_path);
    if (!loader.is_valid()) {
        std::cerr << "Error: Could not open data file: " << data_path << "\n";
        return 1;
    }
    
    IndicatorEngine indicators;
    StrategyEngine strategy(config);
    ExecutionSimulator executor(config);
    RegimeDetector regime_detector;
    Analytics analytics;
    
    executor.reset(100000.0); // Initial cash
    analytics.reset();
    
    std::cout << "Running backtest: " << config.name << "\n";
    std::cout << "Data file: " << data_path << "\n";
    std::cout << "Processing ticks...\n";
    
    size_t tick_count = 0;
    
    // Main event loop
    while (loader.has_next()) {
        OHLCV tick = loader.next();
        if (tick.close <= 0.0) continue; // Skip invalid ticks
        
        tick_count++;
        
        // Update indicators
        indicators.add_price(tick.close, tick.volume);
        
        // Update regime detector
        Regime current_regime = regime_detector.update_and_get(tick);

        // Skip trading in volatile regime if configured
        if (config.exclude_volatile_regime && current_regime == Regime::VOLATILE) {
            continue;
        }
        
        // Get strategy signals
        std::vector<Order> orders = strategy.on_tick(tick, indicators);
        
        // Execute orders
        for (const auto& order : orders) {
            double realized_vol = indicators.get_realized_vol(20);
            Fill fill = executor.execute(order, tick, realized_vol);
            
            // Record fill in analytics
            double position_value = executor.get_position().size * tick.close;
            analytics.record_fill(fill, current_regime, executor.get_cash(), position_value);
        }
        
        // Progress indicator
        if (tick_count % 1000 == 0) {
            std::cout << "Processed " << tick_count << " ticks...\n";
        }
    }
    
    std::cout << "Completed processing " << tick_count << " ticks.\n";
    
    // Generate summary
    BacktestSummary summary = analytics.summary();
    print_summary(summary);
    
    // Export results
    if (!output_path.empty()) {
        analytics.export_summary_json(output_path);
        
        // Export trade log to CSV (same directory, different extension)
        std::string trade_log_path = output_path;
        size_t last_dot = trade_log_path.find_last_of('.');
        if (last_dot != std::string::npos) {
            trade_log_path = trade_log_path.substr(0, last_dot) + "_trades.csv";
        } else {
            trade_log_path += "_trades.csv";
        }
        analytics.export_trade_log(trade_log_path);
        std::cout << "\nResults exported to:\n";
        std::cout << "  Summary: " << output_path << "\n";
        std::cout << "  Trades:  " << trade_log_path << "\n";
    }
    
    return 0;
}

int benchmark_mode(const std::string& strategy_path, const std::string& data_path, int parallel) {
    std::cout << "Benchmark mode not yet implemented.\n";
    std::cout << "This would run parameter sweeps with " << parallel << " parallel threads.\n";
    return 0;
}

int stats_mode(const std::string& results_path) {
    std::ifstream file(results_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open results file: " << results_path << "\n";
        return 1;
    }
    
    std::cout << "Reading results from: " << results_path << "\n";
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << "\n";
    }
    file.close();
    
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "run") {
        std::string strategy_path, data_path, output_path;
        
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--strategy" && i + 1 < argc) {
                strategy_path = argv[++i];
            } else if (arg == "--data" && i + 1 < argc) {
                data_path = argv[++i];
            } else if (arg == "--out" && i + 1 < argc) {
                output_path = argv[++i];
            }
        }
        
        if (strategy_path.empty() || data_path.empty()) {
            std::cerr << "Error: --strategy and --data are required.\n";
            print_usage();
            return 1;
        }
        
        return run_backtest(strategy_path, data_path, output_path);
        
    } else if (command == "benchmark") {
        std::string strategy_path, data_path;
        int parallel = 1;
        
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--strategy" && i + 1 < argc) {
                strategy_path = argv[++i];
            } else if (arg == "--data" && i + 1 < argc) {
                data_path = argv[++i];
            } else if (arg == "--parallel" && i + 1 < argc) {
                parallel = std::stoi(argv[++i]);
            }
        }
        
        if (strategy_path.empty() || data_path.empty()) {
            std::cerr << "Error: --strategy and --data are required.\n";
            print_usage();
            return 1;
        }
        
        return benchmark_mode(strategy_path, data_path, parallel);
        
    } else if (command == "stats") {
        std::string results_path;
        
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--results" && i + 1 < argc) {
                results_path = argv[++i];
            }
        }
        
        if (results_path.empty()) {
            std::cerr << "Error: --results is required.\n";
            print_usage();
            return 1;
        }
        
        return stats_mode(results_path);
        
    } else {
        std::cerr << "Error: Unknown command: " << command << "\n\n";
        print_usage();
        return 1;
    }
}

