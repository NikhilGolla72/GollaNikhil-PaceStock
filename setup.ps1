# FluxBack Setup Script
# Checks dependencies and prepares the project

Write-Host "=== FluxBack Setup ===" -ForegroundColor Cyan

# Check for CMake
$cmakeFound = $false
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeVersion = cmake --version | Select-Object -First 1
    Write-Host "CMake found: $cmakeVersion" -ForegroundColor Green
    $cmakeFound = $true
} else {
    Write-Host "CMake not found" -ForegroundColor Red
    Write-Host "  Please install CMake from: https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host "  Or use: winget install Kitware.CMake" -ForegroundColor Yellow
}

# Check for C++ compiler
$compilerFound = $false
if (Get-Command cl -ErrorAction SilentlyContinue) {
    Write-Host "MSVC compiler found" -ForegroundColor Green
    $compilerFound = $true
} elseif (Get-Command g++ -ErrorAction SilentlyContinue) {
    Write-Host "GCC compiler found" -ForegroundColor Green
    $compilerFound = $true
} else {
    Write-Host "C++ compiler not found" -ForegroundColor Red
    Write-Host "  Please install Visual Studio Build Tools or MinGW" -ForegroundColor Yellow
}

# Prepare data
Write-Host ""
Write-Host "=== Preparing Data ===" -ForegroundColor Cyan
if (Test-Path "scripts\prepare_data.ps1") {
    .\scripts\prepare_data.ps1
} else {
    Write-Host "Data preparation script not found" -ForegroundColor Red
}

# Build instructions
Write-Host ""
Write-Host "=== Build Instructions ===" -ForegroundColor Cyan
if ($cmakeFound -and $compilerFound) {
    Write-Host "To build the project, run:" -ForegroundColor Green
    Write-Host "  mkdir build" -ForegroundColor White
    Write-Host "  cd build" -ForegroundColor White
    Write-Host "  cmake .. -DCMAKE_BUILD_TYPE=Release" -ForegroundColor White
    Write-Host "  cmake --build . --config Release" -ForegroundColor White
    Write-Host ""
    Write-Host "To run a backtest:" -ForegroundColor Green
    Write-Host "  .\fluxback.exe run --strategy ..\config\sma_demo.yaml --data ..\demo\aapl_sample.csv --out ..\results\demo.json" -ForegroundColor White
} else {
    Write-Host "Please install missing dependencies first." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== Setup Complete ===" -ForegroundColor Cyan
