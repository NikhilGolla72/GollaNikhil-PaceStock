// FluxBack Web Interface
class FluxBackApp {
    constructor() {
        this.init();
    }

    init() {
        this.setupEventListeners();
        this.loadSampleData();
    }

    setupEventListeners() {
        document.getElementById('runBacktest').addEventListener('click', () => this.runBacktest());
        document.getElementById('uploadData').addEventListener('change', (e) => this.handleFileUpload(e));
        document.getElementById('loadSample').addEventListener('click', () => this.loadSampleData());
    }

    loadSampleData() {
        // Load sample CSV data
        fetch('demo/aapl_sample.csv')
            .then(response => response.text())
            .then(data => {
                document.getElementById('csvData').value = data;
            })
            .catch(() => {
                document.getElementById('csvData').value = `timestamp,open,high,low,close,volume
2024-01-02T09:15:00,100.50,101.20,100.10,100.90,15000
2024-01-02T09:16:00,100.90,101.50,100.80,101.30,12000`;
            });

        // Load sample strategy
        fetch('config/sma_demo.yaml')
            .then(response => response.text())
            .then(data => {
                document.getElementById('strategyYaml').value = data;
            })
            .catch(() => {
                document.getElementById('strategyYaml').value = `strategy:
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
    vol_multiplier: 0.001`;
            });
    }

    handleFileUpload(event) {
        const file = event.target.files[0];
        if (file) {
            const reader = new FileReader();
            reader.onload = (e) => {
                document.getElementById('csvData').value = e.target.result;
            };
            reader.readAsText(file);
        }
    }

    async runBacktest() {
        const button = document.getElementById('runBacktest');
        const resultsDiv = document.getElementById('results');
        
        button.disabled = true;
        button.textContent = 'Running...';
        resultsDiv.innerHTML = '<p>Processing backtest...</p>';

        const strategyYaml = document.getElementById('strategyYaml').value;
        const csvData = document.getElementById('csvData').value;

        try {
            // Try API endpoint first
            const response = await fetch('/api/backtest', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    strategy: strategyYaml,
                    data: csvData
                })
            });

            const result = await response.json();

            if (response.ok) {
                this.displayResults(result);
            } else {
                // If API not available, show local instructions
                this.showLocalInstructions(result);
            }
        } catch (error) {
            console.error('API error:', error);
            this.showLocalInstructions({ error: 'API not available' });
        } finally {
            button.disabled = false;
            button.textContent = 'Run Backtest';
        }
    }

    showLocalInstructions(error) {
        const resultsDiv = document.getElementById('results');
        resultsDiv.innerHTML = `
            <div style="background: #fff3cd; border: 1px solid #ffc107; padding: 15px; border-radius: 5px; margin: 20px 0;">
                <h3>⚠️ Online Backtesting Not Available</h3>
                <p>The C++ backtesting engine needs to run locally on your machine.</p>
                <p><strong>To run locally:</strong></p>
                <ol>
                    <li>Clone the repository: <code>git clone https://github.com/NikhilGolla72/GollaNikhil-PaceStock.git</code></li>
                    <li>Build the project: <code>cd GollaNikhil-PaceStock && ./build.ps1</code></li>
                    <li>Run: <code>./run_demo.ps1</code></li>
                </ol>
                <p><strong>Or use the data below to run manually:</strong></p>
                <details>
                    <summary>Click to see command</summary>
                    <pre style="background: #f5f5f5; padding: 10px; margin-top: 10px;">
cd build\\Release
.\\fluxback.exe run --strategy ..\\..\\config\\sma_demo.yaml --data ..\\..\\demo\\aapl_sample.csv --out ..\\..\\results\\demo.json</pre>
                </details>
            </div>
        `;
    }

    displayResults(result) {
        const resultsDiv = document.getElementById('results');
        
        const html = `
            <div style="background: #d4edda; border: 1px solid #28a745; padding: 20px; border-radius: 5px; margin: 20px 0;">
                <h2>✅ Backtest Results</h2>
                
                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-top: 20px;">
                    <div style="background: white; padding: 15px; border-radius: 5px;">
                        <strong>Total Return</strong><br>
                        <span style="font-size: 1.5em; color: ${result.total_return_pct >= 0 ? '#28a745' : '#dc3545'}">
                            ${result.total_return_pct.toFixed(2)}%
                        </span>
                    </div>
                    
                    <div style="background: white; padding: 15px; border-radius: 5px;">
                        <strong>Sharpe Ratio</strong><br>
                        <span style="font-size: 1.5em;">${result.sharpe_ratio.toFixed(2)}</span>
                    </div>
                    
                    <div style="background: white; padding: 15px; border-radius: 5px;">
                        <strong>Win Rate</strong><br>
                        <span style="font-size: 1.5em;">${result.win_rate_pct.toFixed(2)}%</span>
                    </div>
                    
                    <div style="background: white; padding: 15px; border-radius: 5px;">
                        <strong>Total Trades</strong><br>
                        <span style="font-size: 1.5em;">${result.total_trades}</span>
                    </div>
                    
                    <div style="background: white; padding: 15px; border-radius: 5px;">
                        <strong>Max Drawdown</strong><br>
                        <span style="font-size: 1.5em; color: #dc3545">${result.max_drawdown_pct.toFixed(2)}%</span>
                    </div>
                    
                    <div style="background: white; padding: 15px; border-radius: 5px;">
                        <strong>Profit Factor</strong><br>
                        <span style="font-size: 1.5em;">${result.profit_factor.toFixed(2)}</span>
                    </div>
                </div>
                
                <div style="margin-top: 20px;">
                    <h3>Trade Statistics</h3>
                    <p>Winning Trades: ${result.winning_trades} | Losing Trades: ${result.losing_trades}</p>
                    <p>Initial Cash: $${result.initial_cash.toFixed(2)} → Final Cash: $${result.final_cash.toFixed(2)}</p>
                </div>
            </div>
        `;
        
        resultsDiv.innerHTML = html;
    }
}

// Initialize app when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => new FluxBackApp());
} else {
    new FluxBackApp();
}

