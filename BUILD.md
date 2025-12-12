# Building FluxBack

## Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Python 3.6+ (for Python bindings)
- pybind11 (for Python bindings, optional)

## Building on Linux/macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Building with Python Bindings

```bash
# Install pybind11 first
pip install pybind11

# Build with Python bindings enabled
mkdir build && cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON
cmake --build . --config Release
```

After building, add the Python module to your path:
```bash
export PYTHONPATH=$PWD/build/python:$PYTHONPATH
```

## Building on Windows

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## Running Tests

```bash
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .
ctest
```

## Running the Demo

```bash
# From the build directory
./fluxback run --strategy ../config/sma_demo.yaml --data ../demo/sample_data.csv --out ../results/demo.json
```

## Docker Build

```bash
docker build -t fluxback .
docker run -it fluxback
```

