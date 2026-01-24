#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
    std::string sensorID;
    double period;
    double targetTemp;
    bool verbose;
    std::string pidFile;
    bool killDaemon;
    
    // Default values
    static constexpr double DEFAULT_PERIOD = 1.0;
    static constexpr double DEFAULT_TARGET_TEMP = 80.0;
    
    // Parse command line arguments
    // Returns true on success, false on error (usage will be printed)
    static bool parse(int argc, char** argv, Config& config);
    
    // Validate configuration
    // Returns true if valid, false otherwise
    bool validate() const;
    
    // Show usage information
    static void showUsage(const char* programName, const std::string& message = "");
    
private:
    static void showAvailableSensors();
};

#endif // CONFIG_HPP
