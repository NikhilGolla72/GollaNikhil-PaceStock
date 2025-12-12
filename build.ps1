# FluxBack Build Script
# Automatically adds CMake to PATH and builds the project

# Add CMake to PATH if not already there
$cmakePath = "C:\Program Files\CMake\bin"
if (Test-Path $cmakePath) {
    if ($env:Path -notlike "*$cmakePath*") {
        $env:Path += ";$cmakePath"
        Write-Host "Added CMake to PATH for this session" -ForegroundColor Green
    }
}

# Check if CMake is available
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "Error: CMake not found. Please install CMake and add it to PATH." -ForegroundColor Red
    Write-Host "  Download from: https://cmake.org/download/" -ForegroundColor Yellow
    exit 1
}

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
    Write-Host "Created build directory" -ForegroundColor Green
}

# Configure
Write-Host "`nConfiguring project..." -ForegroundColor Cyan
Set-Location build
cmake .. -DCMAKE_BUILD_TYPE=Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

# Build
Write-Host "`nBuilding project..." -ForegroundColor Cyan
cmake --build . --config Release

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild successful!" -ForegroundColor Green
    Write-Host "`nTo run a backtest:" -ForegroundColor Yellow
    Write-Host "  cd Release" -ForegroundColor White
    Write-Host "  .\fluxback.exe run --strategy ..\..\config\sma_demo.yaml --data ..\..\demo\aapl_sample.csv --out ..\..\results\demo.json" -ForegroundColor White
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
}

Set-Location ..

