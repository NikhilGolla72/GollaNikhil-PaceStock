#include "data/DataLoader.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace fluxback {

DataLoader::DataLoader(const std::string& csv_path)
    : csv_path(csv_path), current_line(0), total_lines(0), header_read(false) {
    file_stream.open(csv_path);
    if (file_stream.is_open()) {
        // Skip header
        std::string header;
        if (std::getline(file_stream, header)) {
            header_read = true;
        }
        count_total_lines();
        reset();
    }
}

DataLoader::~DataLoader() {
    if (file_stream.is_open()) {
        file_stream.close();
    }
}

void DataLoader::count_total_lines() {
    std::ifstream temp_stream(csv_path);
    if (!temp_stream.is_open()) return;
    
    std::string line;
    total_lines = 0;
    bool skip_header = true;
    while (std::getline(temp_stream, line)) {
        if (skip_header) {
            skip_header = false;
            continue;
        }
        if (!line.empty() && line.find(',') != std::string::npos) {
            total_lines++;
        }
    }
    temp_stream.close();
}

void DataLoader::reset() {
    if (file_stream.is_open()) {
        file_stream.close();
    }
    file_stream.open(csv_path);
    if (file_stream.is_open()) {
        std::string header;
        std::getline(file_stream, header); // Skip header
        current_line = 0;
        header_read = true;
    }
}

bool DataLoader::has_next() {
    if (!file_stream.is_open()) return false;
    
    // Check if there's a next line
    std::streampos current_pos = file_stream.tellg();
    std::string line;
    bool has_line = static_cast<bool>(std::getline(file_stream, line));
    file_stream.seekg(current_pos);
    
    return has_line && !line.empty();
}

OHLCV DataLoader::next() {
    OHLCV ohlcv;
    
    if (!file_stream.is_open() || !has_next()) {
        return ohlcv;
    }
    
    std::string line;
    if (std::getline(file_stream, line)) {
        if (parse_line(line, ohlcv)) {
            current_line++;
        }
    }
    
    return ohlcv;
}

std::vector<std::string> DataLoader::split_csv_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    
    bool in_quotes = false;
    std::string current_token;
    
    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            tokens.push_back(current_token);
            current_token.clear();
        } else {
            current_token += c;
        }
    }
    tokens.push_back(current_token);
    
    return tokens;
}

bool DataLoader::parse_line(const std::string& line, OHLCV& ohlcv) {
    if (line.empty()) return false;
    
    auto tokens = split_csv_line(line);
    if (tokens.size() < 6) return false;
    
    try {
        ohlcv.timestamp = tokens[0];
        ohlcv.open = std::stod(tokens[1]);
        ohlcv.high = std::stod(tokens[2]);
        ohlcv.low = std::stod(tokens[3]);
        ohlcv.close = std::stod(tokens[4]);
        ohlcv.volume = std::stol(tokens[5]);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing line: " << line << " - " << e.what() << std::endl;
        return false;
    }
}

} // namespace fluxback

