# Quick demo runner script
# Runs a backtest with the sample data

$buildDir = "build\Release"
$exe = "$buildDir\fluxback.exe"

if (-not (Test-Path $exe)) {
    Write-Host "Error: fluxback.exe not found. Please build the project first:" -ForegroundColor Red
    Write-Host "  .\build.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "Running FluxBack Demo..." -ForegroundColor Cyan
Write-Host ""

& $exe run --strategy config\sma_demo.yaml --data demo\aapl_sample.csv --out results\demo.json

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nResults saved to:" -ForegroundColor Green
    Write-Host "  results\demo.json" -ForegroundColor White
    Write-Host "  results\demo_trades.csv" -ForegroundColor White
}

