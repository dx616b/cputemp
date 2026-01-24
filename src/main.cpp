#include "config.hpp"
#include "temperature_sensor.hpp"
#include "frequency_controller.hpp"
#include "pid_controller.hpp"
#include "daemon_manager.hpp"
#include "logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <cstdio>

// Global for signal handler and atexit to reset frequency
static void resetFrequencyToMax() {
    double maxFreq = FrequencyController::getMaxFreq();
    if (maxFreq > 0) {
        FrequencyController::setMaxFrequency(maxFreq);
    }
}

// Atexit handler to ensure frequency is reset on normal exit
static void atexitResetFrequency() {
    resetFrequencyToMax();
}

int main(int argc, char** argv) {
    Config config;
    if (!Config::parse(argc, argv, config)) {
        return -1;
    }
    
    // Use syslog in daemon mode, stdout/stderr otherwise
    // Use syslog in daemon mode, stdout/stderr otherwise
    bool useSyslog = !config.pidFile.empty();
    Logger logger(config.verbose, useSyslog);
    
    // Handle kill-daemon option
    if (config.killDaemon) {
        DaemonManager daemonMgr;
        if (daemonMgr.killDaemon(config.pidFile)) {
            return 0;
        }
        logger.error("Failed to kill daemon");
        return -1;
    }
    
    // Handle daemon mode setup (before sensor discovery)
    DaemonManager daemonMgr;
    bool daemonMode = !config.pidFile.empty();
    
    if (daemonMode) {
        // Open PID file and kill existing daemon if needed
        if (!daemonMgr.openPidFile(config.pidFile, true)) {
            return -1;
        }
        // Close temporarily - will reopen after validation
        daemonMgr.closePidFile();
    }
    
    // Find temperature sensor
    TemperatureSensor sensor;
    if (!TemperatureSensor::findSensor(config.sensorID, sensor)) {
        return -1;
    }
    
    logger.info("Sensor file name : " + sensor.getPath());
    
    // Get frequency limits
    auto freqLimits = FrequencyController::getMinMaxFreq();
    if (freqLimits.first < 0 || freqLimits.second < 0) {
        logger.error("Could not determine CPU frequency limits");
        return -1;
    }
    
    const double minFreq = freqLimits.first;
    const double maxFreq = freqLimits.second;
    
    if (logger.isVerbose()) {
        logger.info("Max freq : " + std::to_string(maxFreq / 1000000.0) + " MHz");
        logger.info("Min freq : " + std::to_string(minFreq / 1000000.0) + " MHz");
        logger.info("Cur freq : " + std::to_string(FrequencyController::getCurrentFreq() / 1000000.0) + " MHz");
    }
    
    // Validate frequency control
    if (!FrequencyController::validateControl()) {
        return -1;
    }
    
    // Setup daemon mode (after validation)
    if (daemonMode) {
        // Reopen PID file after validation
        if (!daemonMgr.openPidFile(config.pidFile, false)) {
            return -1;
        }
        
        if (!daemonMgr.daemonize()) {
            return -1;
        }
        
        // Setup signal handler to reset frequency on exit
        daemonMgr.setupSignalHandler(resetFrequencyToMax);
        
        // Register atexit handler to ensure frequency reset on normal exit
        std::atexit(atexitResetFrequency);
        
        daemonMgr.writePid();
        
        logger.info("cputemp daemon started, target temperature: " + std::to_string(config.targetTemp) + " C");
    } else {
        // Even in non-daemon mode, register atexit for cleanup
        std::atexit(atexitResetFrequency);
    }
    
    // Initialize PID controller
    // Constants from original code: C1 = C2 = 20000000.0 * period
    const double C1 = 20000000.0 * config.period;
    const double C2 = 20000000.0 * config.period;
    PIDController pidController(C1, C2, minFreq, maxFreq);
    
    // Initialize with current frequency
    double currentFreq = FrequencyController::getMaxScalingFreq();
    pidController.reset(currentFreq);
    
    // Read initial temperature
    auto tempOpt = sensor.readTemperature();
    if (!tempOpt.has_value()) {
        logger.error("Could not read initial temperature");
        return -1;
    }
    double prevTemp = tempOpt.value();
    
    // Main control loop
    for (;;) {
        auto tempOpt = sensor.readTemperature();
        if (!tempOpt.has_value()) {
            logger.error("Could not read temperature");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        
        double curTemp = tempOpt.value();
        
        // Store current frequency before calculation
        double oldFreq = currentFreq;
        
        // Calculate next frequency using PID controller
        double nextFreq = pidController.calculate(curTemp, config.targetTemp, prevTemp);
        currentFreq = nextFreq;
        
        // Detect throttling: temperature exceeds target AND frequency is reduced
        bool isThrottling = (curTemp > config.targetTemp) && (nextFreq < oldFreq);
        bool isIncreasing = (curTemp < config.targetTemp) && (nextFreq > oldFreq);
        
        // Set the frequency
        int coresSet = FrequencyController::setMaxFrequency(nextFreq);
        if (coresSet <= 0) {
            logger.error("Failed to set CPU frequency");
        }
        
        // Log throttling events (always logged, even if not verbose)
        if (isThrottling) {
            logger.logThrottle(curTemp, config.targetTemp, oldFreq, nextFreq);
        } else if (isIncreasing && logger.isVerbose()) {
            logger.logFrequencyIncrease(curTemp, config.targetTemp, oldFreq, nextFreq);
        }
        
        // Log verbose information
        if (logger.isVerbose()) {
            double curActualFreq = FrequencyController::getCurrentFreq();
            double curMaxFreq = FrequencyController::getMaxScalingFreq();
            logger.verbose("CPU freq = " + std::to_string(curActualFreq / 1000000.0) + " MHz, ");
            logger.verbose("scaling_max_freq = " + std::to_string(curMaxFreq / 1000000.0) + " MHz, ");
            logger.verbose("CPU temp = " + std::to_string(curTemp) + " C, ");
            logger.info("target temp = " + std::to_string(config.targetTemp) + " C");
        }
        
        prevTemp = curTemp;
        
        // Sleep for the specified period
        std::this_thread::sleep_for(std::chrono::microseconds(
            static_cast<long>(config.period * 1e6)));
    }
    
    return 0;
}
