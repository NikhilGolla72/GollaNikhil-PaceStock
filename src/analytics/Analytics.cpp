#include "analytics/Analytics.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

namespace fluxback {

Analytics::Analytics()
    : initial_cash(100000.0), current_cash(100000.0), peak_equity(100000.0), max_drawdown(0.0) {
    open_position.is_open = false;
}

void Analytics::reset() {
    fills.clear();
    trades.clear();
    equity_curve.clear();
    initial_cash = 100000.0;
    current_cash = 100000.0;
    peak_equity = 100000.0;
    max_drawdown = 0.0;
    open_position.is_open = false;
}

void Analytics::record_fill(const Fill& fill, Regime regime, double current_cash, double current_position_value) {
    fills.push_back(fill);
    this->current_cash = current_cash;
    
    double current_equity = current_cash + current_position_value;
    equity_curve.push_back({fill.timestamp, current_equity});
    update_drawdown(current_equity);
    
    // Handle position tracking
    if (!open_position.is_open) {
        // Opening a new position
        if ((fill.order.type == Order::BUY && fill.order.size > 0) ||
            (fill.order.type == Order::SELL && fill.order.size > 0)) {
            open_position.entry_fill = fill;
            open_position.entry_regime = regime;
            open_position.is_open = true;
        }
    } else {
        // Check if this fill closes the position
        bool closes_position = false;
        
        if (open_position.entry_fill.order.type == Order::BUY) {
            // Long position: closes on SELL
            closes_position = (fill.order.type == Order::SELL);
        } else {
            // Short position: closes on BUY
            closes_position = (fill.order.type == Order::BUY);
        }
        
        if (closes_position) {
            close_trade(fill, regime);
            open_position.is_open = false;
        }
    }
}

void Analytics::close_trade(const Fill& exit_fill, Regime exit_regime) {
    Trade trade;
    trade.entry_timestamp = open_position.entry_fill.timestamp;
    trade.exit_timestamp = exit_fill.timestamp;
    trade.entry_price = open_position.entry_fill.fill_price;
    trade.exit_price = exit_fill.fill_price;
    trade.entry_regime = open_position.entry_regime;
    trade.exit_regime = exit_regime;
    
    // Calculate PnL
    if (open_position.entry_fill.order.type == Order::BUY) {
        // Long position
        trade.size = open_position.entry_fill.filled_size;
        trade.pnl = (exit_fill.fill_price - open_position.entry_fill.fill_price) * trade.size;
        trade.pnl_pct = ((exit_fill.fill_price - open_position.entry_fill.fill_price) / open_position.entry_fill.fill_price) * 100.0;
    } else {
        // Short position
        trade.size = open_position.entry_fill.filled_size;
        trade.pnl = (open_position.entry_fill.fill_price - exit_fill.fill_price) * trade.size;
        trade.pnl_pct = ((open_position.entry_fill.fill_price - exit_fill.fill_price) / open_position.entry_fill.fill_price) * 100.0;
    }
    
    trade.is_win = trade.pnl > 0.0;
    trades.push_back(trade);
}

BacktestSummary Analytics::summary() const {
    BacktestSummary s;
    s.initial_cash = initial_cash;
    
    if (equity_curve.empty()) {
        s.final_cash = initial_cash;
        s.total_return_pct = 0.0;
    } else {
        s.final_cash = equity_curve.back().second;
        s.total_return_pct = ((s.final_cash - initial_cash) / initial_cash) * 100.0;
    }
    
    // Annualized return (assuming ~252 trading days, ~390 minutes per day)
    // For 1-month demo: ~20 trading days = ~7800 minutes
    // This is a simplification - in production, calculate actual time span
    double time_span_years = 1.0 / 12.0; // Assume 1 month for demo
    if (time_span_years > 0) {
        s.annualized_return_pct = (std::pow(s.final_cash / initial_cash, 1.0 / time_span_years) - 1.0) * 100.0;
    } else {
        s.annualized_return_pct = s.total_return_pct;
    }
    
    s.total_trades = trades.size();
    s.winning_trades = 0;
    s.losing_trades = 0;
    double total_win = 0.0;
    double total_loss = 0.0;
    
    for (const auto& trade : trades) {
        if (trade.is_win) {
            s.winning_trades++;
            total_win += trade.pnl;
        } else {
            s.losing_trades++;
            total_loss += std::abs(trade.pnl);
        }
        
        // Per-regime stats
        s.trades_by_regime[trade.entry_regime]++;
        s.pnl_by_regime[trade.entry_regime] += trade.pnl;
    }
    
    s.win_rate_pct = s.total_trades > 0 ? (static_cast<double>(s.winning_trades) / s.total_trades) * 100.0 : 0.0;
    s.avg_win_pct = s.winning_trades > 0 ? total_win / s.winning_trades : 0.0;
    s.avg_loss_pct = s.losing_trades > 0 ? total_loss / s.losing_trades : 0.0;
    s.profit_factor = s.losing_trades > 0 && total_loss > 0 ? total_win / total_loss : 0.0;
    
    s.sharpe_ratio = calculate_sharpe_ratio();
    s.max_drawdown_pct = max_drawdown;
    s.equity_curve = equity_curve;
    
    return s;
}

double Analytics::calculate_sharpe_ratio() const {
    if (equity_curve.size() < 2) return 0.0;
    
    std::vector<double> returns;
    for (size_t i = 1; i < equity_curve.size(); ++i) {
        if (equity_curve[i-1].second > 0.0) {
            double ret = (equity_curve[i].second - equity_curve[i-1].second) / equity_curve[i-1].second;
            returns.push_back(ret);
        }
    }
    
    if (returns.empty()) return 0.0;
    
    double mean = 0.0;
    for (double ret : returns) {
        mean += ret;
    }
    mean /= returns.size();
    
    double variance = 0.0;
    for (double ret : returns) {
        double diff = ret - mean;
        variance += diff * diff;
    }
    variance /= returns.size();
    double stddev = std::sqrt(variance);
    
    if (stddev == 0.0) return 0.0;
    
    // Annualized Sharpe (assuming 252 trading days)
    return (mean / stddev) * std::sqrt(252.0);
}

void Analytics::update_drawdown(double current_equity) {
    if (current_equity > peak_equity) {
        peak_equity = current_equity;
    }
    
    double drawdown = ((peak_equity - current_equity) / peak_equity) * 100.0;
    if (drawdown > max_drawdown) {
        max_drawdown = drawdown;
    }
}

void Analytics::export_trade_log(const std::string& csv_path) const {
    std::ofstream file(csv_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << csv_path << std::endl;
        return;
    }
    
    // Header
    file << "entry_timestamp,exit_timestamp,entry_price,exit_price,size,pnl,pnl_pct,entry_regime,exit_regime,is_win\n";
    
    // Data
    for (const auto& trade : trades) {
        file << trade.entry_timestamp << ","
             << trade.exit_timestamp << ","
             << std::fixed << std::setprecision(2) << trade.entry_price << ","
             << trade.exit_price << ","
             << trade.size << ","
             << trade.pnl << ","
             << std::setprecision(4) << trade.pnl_pct << ","
             << regime_to_string(trade.entry_regime) << ","
             << regime_to_string(trade.exit_regime) << ","
             << (trade.is_win ? "1" : "0") << "\n";
    }
    
    file.close();
}

void Analytics::export_summary_json(const std::string& json_path) const {
    auto s = summary();
    
    std::ofstream file(json_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << json_path << std::endl;
        return;
    }
    
    file << std::fixed << std::setprecision(4);
    file << "{\n";
    file << "  \"total_return_pct\": " << s.total_return_pct << ",\n";
    file << "  \"annualized_return_pct\": " << s.annualized_return_pct << ",\n";
    file << "  \"sharpe_ratio\": " << s.sharpe_ratio << ",\n";
    file << "  \"max_drawdown_pct\": " << s.max_drawdown_pct << ",\n";
    file << "  \"total_trades\": " << s.total_trades << ",\n";
    file << "  \"winning_trades\": " << s.winning_trades << ",\n";
    file << "  \"losing_trades\": " << s.losing_trades << ",\n";
    file << "  \"win_rate_pct\": " << s.win_rate_pct << ",\n";
    file << "  \"avg_win_pct\": " << s.avg_win_pct << ",\n";
    file << "  \"avg_loss_pct\": " << s.avg_loss_pct << ",\n";
    file << "  \"profit_factor\": " << s.profit_factor << ",\n";
    file << "  \"initial_cash\": " << s.initial_cash << ",\n";
    file << "  \"final_cash\": " << s.final_cash << "\n";
    file << "}\n";
    
    file.close();
}

std::string Analytics::regime_to_string(Regime r) const {
    switch (r) {
        case Regime::TREND: return "TREND";
        case Regime::VOLATILE: return "VOLATILE";
        case Regime::SIDEWAYS: return "SIDEWAYS";
        default: return "UNKNOWN";
    }
}

} // namespace fluxback

