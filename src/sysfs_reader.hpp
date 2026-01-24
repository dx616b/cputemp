#ifndef SYSFS_READER_HPP
#define SYSFS_READER_HPP

#include <string>
#include <optional>
#include <cstdint>

class SysfsReader {
public:
    // Read a numeric value from a sysfs file
    // Returns empty optional on error
    static std::optional<int64_t> readNumber(const std::string& filename);
    
    // Read a string value from a sysfs file
    // Returns empty optional on error
    static std::optional<std::string> readString(const std::string& filename);
    
    // Write a numeric value to a sysfs file
    // Returns true on success, false on error
    static bool writeNumber(const std::string& filename, int64_t value);
    
    // Write a string value to a sysfs file
    // Returns true on success, false on error
    static bool writeString(const std::string& filename, const std::string& value);
    
    // Check if a file exists and is readable
    static bool fileExists(const std::string& filename);
    
    // Check if a file exists and is writable
    static bool fileWritable(const std::string& filename);
};

#endif // SYSFS_READER_HPP
