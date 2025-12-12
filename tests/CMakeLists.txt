cmake_minimum_required(VERSION 3.15)

# Find Catch2
find_package(Catch2 REQUIRED)

# Test executable
add_executable(test_indicators test_indicator.cpp
    ../src/indicators/IndicatorEngine.cpp
)

target_include_directories(test_indicators PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link Catch2
target_link_libraries(test_indicators Catch2::Catch2)

# Register test
enable_testing()
add_test(NAME IndicatorTests COMMAND test_indicators)

