#include "config.hpp"
#include "sysfs_reader.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>

void Config::showAvailableSensors() {
    std::cerr << "Available sensors : ";
    
    // Check thermal zones
    for (int i = 0; ; i++) {
        std::string path = "/sys/class/thermal/thermal_zone" + std::to_string(i) + "/type";
        auto type = SysfsReader::readString(path);
        if (!type.has_value()) break;
        
        std::string tempPath = "/sys/class/thermal/thermal_zone" + std::to_string(i) + "/temp";
        auto temp = SysfsReader::readNumber(tempPath);
        if (temp.has_value()) {
            std::cerr << type.value() << " ( " << (temp.value() * 0.001) << " C), ";
        }
    }
    
    // Check hwmon devices
    for (int i = 0; ; i++) {
        std::string path = "/sys/class/hwmon/hwmon" + std::to_string(i) + "/name";
        auto name = SysfsReader::readString(path);
        if (!name.has_value()) break;
        
        std::string tempPath = "/sys/class/hwmon/hwmon" + std::to_string(i) + "/temp1_input";
        auto temp = SysfsReader::readNumber(tempPath);
        if (temp.has_value()) {
            std::cerr << name.value() << " (" << (temp.value() * 0.001) << " C), ";
        }
    }
    
    std::cerr << std::endl;
}

void Config::showUsage(const char* programName, const std::string& message) {
    if (!message.empty()) {
        std::cerr << message << std::endl << std::endl;
    }
    
    std::cerr << "Usage : " << programName << " [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "This utility controls the CPU frequency to make its temperature close to the target" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options :" << std::endl;
    std::cerr << "  --sensor <sensor name>         Specify sensor name" << std::endl;
    std::cerr << "  --period <seconds>             Specify period" << std::endl;
    std::cerr << "  --temp <target temperature>    Specify target CPU temperature" << std::endl;
    std::cerr << "  --daemon <pid file name>       Daemonize" << std::endl;
    std::cerr << "  --kill-daemon <pid file name>  Kill already running daemon" << std::endl;
    std::cerr << "  --verbose                      Turn on verbose mode" << std::endl;
    std::cerr << std::endl;
    
    showAvailableSensors();
    
    std::exit(-1);
}

bool Config::parse(int argc, char** argv, Config& config) {
    // Initialize with defaults
    config.sensorID = "";
    config.period = DEFAULT_PERIOD;
    config.targetTemp = DEFAULT_TARGET_TEMP;
    config.verbose = false;
    config.pidFile = "";
    config.killDaemon = false;
    
    // Note: Original code required argc < 2, but options are actually optional
    // We'll allow running without arguments to show usage
    if (argc < 2) {
        showUsage(argv[0], "");
        return false;
    }
    
    int nextArg;
    for (nextArg = 1; nextArg < argc; nextArg++) {
        std::string arg = argv[nextArg];
        
        if (arg == "--sensor") {
            if (nextArg + 1 >= argc) {
                showUsage(argv[0]);
                return false;
            }
            config.sensorID = argv[nextArg + 1];
            nextArg++;
        } else if (arg == "--period") {
            if (nextArg + 1 >= argc) {
                showUsage(argv[0]);
                return false;
            }
            char* endptr;
            config.period = std::strtod(argv[nextArg + 1], &endptr);
            if (endptr == argv[nextArg + 1] || *endptr != '\0' || config.period <= 0) {
                showUsage(argv[0], "A positive real value is expected after --period.");
                return false;
            }
            nextArg++;
        } else if (arg == "--temp") {
            if (nextArg + 1 >= argc) {
                showUsage(argv[0]);
                return false;
            }
            char* endptr;
            config.targetTemp = std::strtod(argv[nextArg + 1], &endptr);
            if (endptr == argv[nextArg + 1] || *endptr != '\0') {
                showUsage(argv[0], "A real value is expected after --temp.");
                return false;
            }
            nextArg++;
        } else if (arg == "--verbose") {
            config.verbose = true;
        } else if (arg == "--daemon") {
            if (nextArg + 1 >= argc) {
                showUsage(argv[0], "Specify pid file name after --daemon");
                return false;
            }
            config.pidFile = argv[nextArg + 1];
            nextArg++;
        } else if (arg == "--kill-daemon") {
            if (nextArg + 1 >= argc) {
                showUsage(argv[0], "Specify pid file name after --kill-daemon");
                return false;
            }
            config.pidFile = argv[nextArg + 1];
            config.killDaemon = true;
            nextArg++;
        } else if (arg.substr(0, 2) == "--") {
            showUsage(argv[0], std::string("Unrecognized option : ") + arg);
            return false;
        } else {
            break;
        }
    }
    
    // Validate incompatible options
    if (!config.pidFile.empty() && config.verbose) {
        showUsage(argv[0], "--daemon and --verbose cannot be specified at a time");
        return false;
    }
    
    return config.validate();
}

bool Config::validate() const {
    if (period <= 0) {
        return false;
    }
    // sensorID can be empty - will be discovered
    // targetTemp can be any value
    return true;
}
