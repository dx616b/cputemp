#include "sysfs_reader.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <climits>

std::optional<int64_t> SysfsReader::readNumber(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    int64_t value;
    if (file >> value) {
        return value;
    }
    
    return std::nullopt;
}

std::optional<std::string> SysfsReader::readString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::string line;
    if (std::getline(file, line)) {
        // Remove trailing newline if present
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
        return line;
    }
    
    return std::nullopt;
}

bool SysfsReader::writeNumber(const std::string& filename, int64_t value) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << value;
    return file.good();
}

bool SysfsReader::writeString(const std::string& filename, const std::string& value) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << value;
    return file.good();
}

bool SysfsReader::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

bool SysfsReader::fileWritable(const std::string& filename) {
    std::ofstream file(filename, std::ios::app);
    return file.good();
}
