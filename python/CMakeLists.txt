cmake_minimum_required(VERSION 3.15)

# Python bindings module
pybind11_add_module(fluxback_py
    bindings.cpp
    ../src/data/DataLoader.cpp
    ../src/indicators/IndicatorEngine.cpp
    ../src/strategy/StrategyEngine.cpp
    ../src/execution/ExecutionSimulator.cpp
    ../src/analytics/Analytics.cpp
    ../src/regime/RegimeDetector.cpp
    ../src/utils/ConfigParser.cpp
)

target_include_directories(fluxback_py PRIVATE ${CMAKE_SOURCE_DIR}/src)

