# Quick Start Guide

## 1. Build the Project

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## 2. Run a Backtest

```bash
# From build directory
./fluxback run --strategy ../config/sma_demo.yaml --data ../demo/sample_data.csv --out ../results/demo.json
```

## 3. View Results

```bash
cat ../results/demo.json
```

## 4. Use Python Bindings

```python
import sys
sys.path.insert(0, 'build/python')
import fluxback

result = fluxback.run_backtest(
    'config/sma_demo.yaml',
    'demo/sample_data.csv'
)

print(f"Total Return: {result['total_return_pct']:.2f}%")
```

## 5. Open Demo Notebook

```bash
jupyter notebook notebooks/demo.ipynb
```

## Next Steps

- Download real market data from [Kaggle](https://www.kaggle.com/datasets/borismarjanovic/price-volume-data-for-all-us-stocks-etfs)
- Create your own strategy YAML files
- Experiment with different indicator parameters
- Analyze per-regime performance

