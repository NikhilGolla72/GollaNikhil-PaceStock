cmake_minimum_required(VERSION 3.15)
project(FluxBack VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(BUILD_TESTS "Build tests" ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(CORE_SOURCES
    src/data/DataLoader.cpp
    src/indicators/IndicatorEngine.cpp
    src/strategy/StrategyEngine.cpp
    src/execution/ExecutionSimulator.cpp
    src/analytics/Analytics.cpp
    src/regime/RegimeDetector.cpp
    src/utils/ConfigParser.cpp
)

# Main executable
add_executable(fluxback
    src/main.cpp
    ${CORE_SOURCES}
)

# Python bindings (if enabled)
if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    if(pybind11_FOUND)
        add_subdirectory(python)
    else()
        message(WARNING "pybind11 not found. Python bindings disabled.")
    endif()
endif()

# Tests (if enabled)
if(BUILD_TESTS)
    enable_testing()
    find_package(Catch2 QUIET)
    if(Catch2_FOUND)
        add_subdirectory(tests)
    else()
        message(WARNING "Catch2 not found. Tests disabled.")
    endif()
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

