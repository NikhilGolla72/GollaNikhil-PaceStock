# WebAssembly Setup for Online Backtesting

To make FluxBack work fully online in the browser, you need to compile the C++ code to WebAssembly.

## Option 1: Emscripten (Recommended)

### Prerequisites
```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh  # On Windows: emsdk_env.bat
```

### Compile to WASM
```bash
mkdir build-wasm
cd build-wasm
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make
```

### Integration
The compiled `.wasm` file can be loaded in the browser and called from JavaScript.

## Option 2: Use Python Backend (Alternative)

Instead of WebAssembly, you can:
1. Deploy Python bindings on a server (Heroku, Railway, etc.)
2. Create API endpoints that call the Python bindings
3. Frontend calls the API

## Option 3: Client-Side Only (Current)

Currently, the web interface shows instructions for local execution. Users download and run locally.

## Quick Implementation Path

For fastest online functionality:
1. Keep current setup (local execution)
2. Add WebAssembly compilation later
3. Or deploy Python API on a different platform (not Vercel)

The current web interface provides a nice UI and falls back to local instructions.

