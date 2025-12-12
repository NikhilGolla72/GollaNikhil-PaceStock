#pragma once

#include <string>
#include <fstream>
#include <vector>

namespace fluxback {

struct OHLCV {
    std::string timestamp;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    long volume = 0;
};

class DataLoader {
public:
    explicit DataLoader(const std::string& csv_path);
    ~DataLoader();
    
    bool has_next();
    OHLCV next();
    void reset();
    
    bool is_valid() const { return file_stream.is_open(); }
    size_t get_current_line() const { return current_line; }
    size_t get_total_lines() const { return total_lines; }

private:
    std::string csv_path;
    std::ifstream file_stream;
    size_t current_line;
    size_t total_lines;
    bool header_read;
    
    void count_total_lines();
    bool parse_line(const std::string& line, OHLCV& ohlcv);
    std::vector<std::string> split_csv_line(const std::string& line);
};

} // namespace fluxback

