# Quick Run Guide - Windows

## Prerequisites

1. **CMake** - Install from https://cmake.org/download/ or run:
   ```powershell
   winget install Kitware.CMake
   ```
   After installation, restart your terminal or run:
   ```powershell
   $env:Path += ";C:\Program Files\CMake\bin"
   ```

2. **C++ Compiler** - Visual Studio Build Tools (already installed if you built before)

## Step-by-Step Instructions

### 1. Open PowerShell in Project Directory

```powershell
cd C:\Users\golla\OneDrive\Desktop\FluxBack
```

### 2. Build the Project

**Option A: Use the build script (Easiest)**
```powershell
.\build.ps1
```

**Option B: Manual build**
```powershell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### 3. Run a Backtest

**Option A: Use the demo script (Easiest)**
```powershell
.\run_demo.ps1
```

**Option B: Manual run**
```powershell
cd build\Release
.\fluxback.exe run --strategy ..\..\config\sma_demo.yaml --data ..\..\demo\aapl_sample.csv --out ..\..\results\demo.json
```

### 4. View Results

After running, check:
- **Summary**: `results\demo.json` - JSON with all metrics
- **Trade Log**: `results\demo_trades.csv` - Detailed trade-by-trade log

## Troubleshooting

### CMake not found
```powershell
# Add CMake to PATH for this session
$env:Path += ";C:\Program Files\CMake\bin"

# Verify
cmake --version
```

### Build errors
- Make sure you're in the project root directory
- Ensure all source files are present in `src/` folder
- Check that Visual Studio Build Tools are installed

### No trades generated
- Check that your data file has enough rows (need at least 20+ for SMA crossover)
- Verify strategy config in `config/sma_demo.yaml`
- Check data format matches expected CSV structure

## Example Output

When successful, you'll see:
```
Running backtest: sma_crossover_demo
Data file: ..\..\demo\aapl_sample.csv
Processing ticks...
Completed processing 199 ticks.

=== Backtest Summary ===
Initial Cash:     $100000.00
Final Cash:       $99980.72
Total Return:     -0.02%
...
```

## Next Steps

- Modify `config/sma_demo.yaml` to test different strategies
- Add your own data files to `demo/` folder
- Experiment with different indicator parameters
- Check `results/demo_trades.csv` for detailed trade analysis

