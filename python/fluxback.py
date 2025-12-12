"""
FluxBack Python convenience wrapper
"""

try:
    import fluxback_py
except ImportError:
    raise ImportError(
        "fluxback_py module not found. Please build the Python bindings first:\n"
        "  mkdir build && cd build\n"
        "  cmake .. -DBUILD_PYTHON_BINDINGS=ON\n"
        "  cmake --build .\n"
        "Then install: pip install -e ."
    )


def run_backtest(strategy_path, data_path):
    """
    Run a backtest with given strategy and data files.
    
    Args:
        strategy_path: Path to YAML strategy configuration file
        data_path: Path to CSV data file (OHLCV format)
    
    Returns:
        dict: Backtest summary with metrics
    """
    return fluxback_py.run_backtest(strategy_path, data_path)


def list_indicators():
    """
    List available technical indicators.
    
    Returns:
        list: List of indicator names and descriptions
    """
    return fluxback_py.list_indicators()


__all__ = ['run_backtest', 'list_indicators']

