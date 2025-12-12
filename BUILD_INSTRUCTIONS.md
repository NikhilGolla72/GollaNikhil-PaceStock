# Build Instructions for FluxBack

## Prerequisites

1. **CMake** (3.15 or higher)
   - Download from: https://cmake.org/download/
   - Or install via: `winget install Kitware.CMake`
   - Add CMake to your PATH during installation

2. **C++ Compiler**
   - **Windows**: Visual Studio 2019 or later (with C++ workload)
   - Or MinGW-w64
   - Make sure compiler is in PATH

## Quick Build

```powershell
# Navigate to project directory
cd C:\Users\golla\OneDrive\Desktop\FluxBack

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release
```

## Run a Backtest

After building, from the `build` directory:

```powershell
.\fluxback.exe run --strategy ..\config\sma_demo.yaml --data ..\demo\aapl_sample.csv --out ..\results\demo.json
```

## Troubleshooting

### CMake not found
- Install CMake and add it to PATH
- Restart terminal after installation

### Compiler not found
- Install Visual Studio Build Tools
- Or install MinGW-w64 and add to PATH

### Build errors
- Make sure you're using C++17 compatible compiler
- Check that all source files are present in `src/` directory

