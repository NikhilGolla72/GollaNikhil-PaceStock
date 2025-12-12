# FluxBack — Regime-Aware C++ Backtesting & Research Engine

**Tagline:** Real-time research meets production performance — a backtester that reasons about market regimes, microstructure, and impact.

## Quick Start

### 1. Install CMake (if not already installed)
```powershell
winget install Kitware.CMake
```
Or download from: https://cmake.org/download/

**Important:** After installing CMake, restart your terminal or run:
```powershell
$env:Path += ";C:\Program Files\CMake\bin"
```

### 2. Prepare Sample Data
The data preparation script will automatically convert your Kaggle dataset:
```powershell
.\scripts\prepare_data.ps1
```

### 3. Build the Project
```powershell
.\build.ps1
```

Or manually:
```powershell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### 4. Run a Demo Backtest
```powershell
.\run_demo.ps1
```

Or manually:
```powershell
cd build\Release
.\fluxback.exe run --strategy ..\..\config\sma_demo.yaml --data ..\..\demo\aapl_sample.csv --out ..\..\results\demo.json
```

## Project Structure

```
fluxback/
├── src/              # C++ source code
│   ├── data/         # DataLoader
│   ├── indicators/   # IndicatorEngine (SMA, EMA, RSI, etc.)
│   ├── strategy/     # StrategyEngine (SMA crossover)
│   ├── execution/    # ExecutionSimulator (slippage model)
│   ├── analytics/    # Analytics (PnL, Sharpe, drawdown)
│   └── regime/       # RegimeDetector (TREND/VOLATILE/SIDEWAYS)
├── config/           # Strategy YAML files
├── demo/             # Sample data files
├── python/           # Python bindings (pybind11)
├── notebooks/        # Jupyter demo notebook
└── results/          # Backtest output files
```

## Features

- **Fast C++ Core**: Event-driven backtesting engine
- **Technical Indicators**: SMA, EMA, RSI, Realized Volatility, VWAP
- **Strategy DSL**: YAML-based strategy configuration
- **Realistic Execution**: Adaptive slippage model based on volatility
- **Regime Detection**: Automatic market regime classification
- **Comprehensive Analytics**: PnL, Sharpe ratio, max drawdown, win rate, per-regime stats
- **Python Bindings**: Use from Jupyter notebooks (optional)

## Example Strategy Configuration

See `config/sma_demo.yaml`:

```yaml
strategy:
  name: sma_crossover_demo
  type: sma_crossover
  symbol: "AAPL"
  timeframe: 1m
  entry:
    fast: 10
    slow: 20
  exit:
    stop_loss_pct: 0.5
    take_profit_pct: 1.0
risk:
  position_size: 100
execution:
  slippage:
    type: adaptive
    base_ticks: 1
    vol_multiplier: 0.001
```

## Data Format

Input CSV should have columns: `timestamp,open,high,low,close,volume`

Example:
```csv
timestamp,open,high,low,close,volume
2024-01-02T09:15:00,100.50,101.20,100.10,100.90,15000
```

## Troubleshooting

### CMake not found
- Install CMake and add to PATH
- Restart terminal after installation
- Or run: `$env:Path += ";C:\Program Files\CMake\bin"`

### Build errors
- Ensure you have Visual Studio Build Tools or MinGW installed
- Check that all source files are present

### Data file not found
- Run `.\scripts\prepare_data.ps1` to prepare sample data
- Or download data from [Kaggle](https://www.kaggle.com/datasets/borismarjanovic/price-volume-data-for-all-us-stocks-etfs)

## License

MIT License — see LICENSE file for details.
