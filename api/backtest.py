"""
Vercel Serverless Function for FluxBack Backtesting API
Note: This requires the Python bindings to be built and available
"""

import json
import sys
import os
from typing import Dict, Any

def handler(request):
    """
    Handle backtest requests
    For Vercel, this will be called via serverless function
    """
    try:
        # Parse request body
        if hasattr(request, 'json'):
            data = request.json
        else:
            data = json.loads(request.body) if hasattr(request, 'body') else {}
        
        strategy_yaml = data.get('strategy', '')
        csv_data = data.get('data', '')
        
        if not strategy_yaml or not csv_data:
            return {
                'statusCode': 400,
                'body': json.dumps({'error': 'Missing strategy or data'})
            }
        
        # Try to import fluxback_py (Python bindings)
        try:
            import fluxback_py
        except ImportError:
            # Fallback: return instructions for local setup
            return {
                'statusCode': 503,
                'body': json.dumps({
                    'error': 'Python bindings not available on server',
                    'message': 'Please run backtests locally or set up Python bindings',
                    'instructions': 'Clone the repo and run: python -m pip install -e .'
                })
            }
        
        # Save temporary files
        import tempfile
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
            f.write(strategy_yaml)
            strategy_path = f.name
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.csv', delete=False) as f:
            f.write(csv_data)
            data_path = f.name
        
        # Run backtest
        result = fluxback_py.run_backtest(strategy_path, data_path)
        
        # Cleanup
        os.unlink(strategy_path)
        os.unlink(data_path)
        
        return {
            'statusCode': 200,
            'body': json.dumps(result)
        }
        
    except Exception as e:
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }

# Vercel serverless function entry point
def main(request):
    return handler(request)

