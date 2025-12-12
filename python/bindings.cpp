#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "data/DataLoader.h"
#include "indicators/IndicatorEngine.h"
#include "strategy/StrategyEngine.h"
#include "execution/ExecutionSimulator.h"
#include "analytics/Analytics.h"
#include "regime/RegimeDetector.h"
#include "utils/ConfigParser.h"
#include <string>
#include <map>

namespace py = pybind11;
using namespace fluxback;

// Helper function to convert Regime enum to string
std::string regime_to_string(Regime r) {
    switch (r) {
        case Regime::TREND: return "TREND";
        case Regime::VOLATILE: return "VOLATILE";
        case Regime::SIDEWAYS: return "SIDEWAYS";
        default: return "UNKNOWN";
    }
}

// Main backtest function exposed to Python
std::map<std::string, py::object> run_backtest(
    const std::string& strategy_path,
    const std::string& data_path
) {
    // Load configuration
    StrategyConfig config = ConfigParser::parse_yaml(strategy_path);
    
    // Initialize components
    DataLoader loader(data_path);
    IndicatorEngine indicators;
    StrategyEngine strategy(config);
    ExecutionSimulator executor(config);
    RegimeDetector regime_detector;
    Analytics analytics;
    
    executor.reset(100000.0);
    analytics.reset();
    
    // Main event loop
    while (loader.has_next()) {
        OHLCV tick = loader.next();
        if (tick.close <= 0.0) continue;
        
        indicators.add_price(tick.close, tick.volume);
        Regime current_regime = regime_detector.update_and_get(tick);
        std::vector<Order> orders = strategy.on_tick(tick, indicators);
        
        for (const auto& order : orders) {
            double realized_vol = indicators.get_realized_vol(20);
            Fill fill = executor.execute(order, tick, realized_vol);
            double position_value = executor.get_position().size * tick.close;
            analytics.record_fill(fill, current_regime, executor.get_cash(), position_value);
        }
    }
    
    // Get summary
    BacktestSummary summary = analytics.summary();
    
    // Convert to Python dictionary
    std::map<std::string, py::object> result;
    result["total_return_pct"] = py::cast(summary.total_return_pct);
    result["annualized_return_pct"] = py::cast(summary.annualized_return_pct);
    result["sharpe_ratio"] = py::cast(summary.sharpe_ratio);
    result["max_drawdown_pct"] = py::cast(summary.max_drawdown_pct);
    result["total_trades"] = py::cast(summary.total_trades);
    result["winning_trades"] = py::cast(summary.winning_trades);
    result["losing_trades"] = py::cast(summary.losing_trades);
    result["win_rate_pct"] = py::cast(summary.win_rate_pct);
    result["avg_win_pct"] = py::cast(summary.avg_win_pct);
    result["avg_loss_pct"] = py::cast(summary.avg_loss_pct);
    result["profit_factor"] = py::cast(summary.profit_factor);
    result["initial_cash"] = py::cast(summary.initial_cash);
    result["final_cash"] = py::cast(summary.final_cash);
    
    // Per-regime stats
    std::map<std::string, int> trades_by_regime;
    std::map<std::string, double> pnl_by_regime;
    for (const auto& [regime, count] : summary.trades_by_regime) {
        trades_by_regime[regime_to_string(regime)] = count;
    }
    for (const auto& [regime, pnl] : summary.pnl_by_regime) {
        pnl_by_regime[regime_to_string(regime)] = pnl;
    }
    result["trades_by_regime"] = py::cast(trades_by_regime);
    result["pnl_by_regime"] = py::cast(pnl_by_regime);
    
    return result;
}

std::vector<std::string> list_indicators() {
    return {
        "SMA - Simple Moving Average",
        "EMA - Exponential Moving Average",
        "RSI - Relative Strength Index",
        "Realized Volatility",
        "VWAP - Volume Weighted Average Price"
    };
}

PYBIND11_MODULE(fluxback_py, m) {
    m.doc() = "FluxBack - Regime-Aware C++ Backtesting Engine";
    
    m.def("run_backtest", &run_backtest, 
          "Run a backtest with given strategy and data files",
          py::arg("strategy_path"), py::arg("data_path"));
    
    m.def("list_indicators", &list_indicators,
          "List available technical indicators");
}

