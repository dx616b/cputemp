#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <syslog.h>

class Logger {
public:
    explicit Logger(bool verbose, bool useSyslog = true) 
        : verbose_(verbose), useSyslog_(useSyslog) {
        if (useSyslog_) {
            openlog("cputemp", LOG_PID | LOG_CONS, LOG_DAEMON);
        }
    }
    
    ~Logger() {
        if (useSyslog_) {
            closelog();
        }
    }
    
    // Log informational message (only in verbose mode)
    void info(const std::string& message) const {
        if (verbose_) {
            if (useSyslog_) {
                syslog(LOG_INFO, "%s", message.c_str());
            } else {
                std::cout << message << std::endl;
            }
        }
    }
    
    // Log error message
    void error(const std::string& message) const {
        if (useSyslog_) {
            syslog(LOG_ERR, "%s", message.c_str());
        } else {
            std::cerr << message << std::endl;
        }
    }
    
    // Log warning message
    void warning(const std::string& message) const {
        if (useSyslog_) {
            syslog(LOG_WARNING, "%s", message.c_str());
        } else {
            std::cerr << "WARNING: " << message << std::endl;
        }
    }
    
    // Log throttling event (always logged, even if not verbose)
    void logThrottle(double temperature, double targetTemp, double oldFreq, double newFreq) const {
        std::string msg = "THROTTLE: Temperature " + std::to_string(temperature) + 
                         " C exceeds target " + std::to_string(targetTemp) + 
                         " C, reducing frequency from " + std::to_string(oldFreq / 1000000.0) + 
                         " MHz to " + std::to_string(newFreq / 1000000.0) + " MHz";
        
        if (useSyslog_) {
            syslog(LOG_WARNING, "%s", msg.c_str());
        } else {
            std::cerr << msg << std::endl;
        }
    }
    
    // Log frequency increase (when temperature is below target)
    void logFrequencyIncrease(double temperature, double targetTemp, double oldFreq, double newFreq) const {
        if (verbose_) {
            std::string msg = "Temperature " + std::to_string(temperature) + 
                             " C below target " + std::to_string(targetTemp) + 
                             " C, increasing frequency from " + std::to_string(oldFreq / 1000000.0) + 
                             " MHz to " + std::to_string(newFreq / 1000000.0) + " MHz";
            
            if (useSyslog_) {
                syslog(LOG_INFO, "%s", msg.c_str());
            } else {
                std::cout << msg << std::endl;
            }
        }
    }
    
    // Log verbose information
    void verbose(const std::string& message) const {
        if (verbose_) {
            if (useSyslog_) {
                syslog(LOG_DEBUG, "%s", message.c_str());
            } else {
                std::cout << message;
            }
        }
    }
    
    // Check if verbose mode is enabled
    bool isVerbose() const { return verbose_; }
    
private:
    bool verbose_;
    bool useSyslog_;
};

#endif // LOGGER_HPP
