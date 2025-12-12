#pragma once

#include "execution/ExecutionSimulator.h"
#include "regime/RegimeDetector.h"
#include <vector>
#include <string>
#include <map>

namespace fluxback {

struct Trade {
    std::string entry_timestamp;
    std::string exit_timestamp;
    double entry_price;
    double exit_price;
    int size;
    double pnl;
    double pnl_pct;
    Regime entry_regime;
    Regime exit_regime;
    bool is_win;
};

struct BacktestSummary {
    double total_return_pct;
    double annualized_return_pct;
    double sharpe_ratio;
    double max_drawdown_pct;
    int total_trades;
    int winning_trades;
    int losing_trades;
    double win_rate_pct;
    double avg_win_pct;
    double avg_loss_pct;
    double profit_factor;
    double initial_cash;
    double final_cash;
    
    // Per-regime statistics
    std::map<Regime, int> trades_by_regime;
    std::map<Regime, double> pnl_by_regime;
    
    // Equity curve data points (timestamp, equity)
    std::vector<std::pair<std::string, double>> equity_curve;
};

class Analytics {
public:
    Analytics();
    
    // Record a fill and update statistics
    void record_fill(const Fill& fill, Regime regime, double current_cash, double current_position_value);
    
    // Get backtest summary
    BacktestSummary summary() const;
    
    // Export trade log to CSV
    void export_trade_log(const std::string& csv_path) const;
    
    // Export summary to JSON (simple format)
    void export_summary_json(const std::string& json_path) const;
    
    // Reset analytics
    void reset();

private:
    std::vector<Fill> fills;
    std::vector<Trade> trades;
    std::vector<std::pair<std::string, double>> equity_curve;
    double initial_cash;
    double current_cash;
    double peak_equity;
    double max_drawdown;
    
    // Track open position for trade construction
    struct OpenPosition {
        Fill entry_fill;
        Regime entry_regime;
        bool is_open;
        
        OpenPosition() : entry_regime(Regime::SIDEWAYS), is_open(false) {}
    };
    OpenPosition open_position;
    
    // Helper methods
    void close_trade(const Fill& exit_fill, Regime exit_regime);
    double calculate_sharpe_ratio() const;
    void update_drawdown(double current_equity);
    std::string regime_to_string(Regime r) const;
};

} // namespace fluxback

