#include "frequency_controller.hpp"
#include "sysfs_reader.hpp"
#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>
#include <cfloat>

double FrequencyController::readCpuFreq(const std::string& filename) {
    auto value = SysfsReader::readNumber(filename);
    if (value.has_value()) {
        // Convert from kHz to Hz
        return value.value() * 1000.0;
    }
    return -1.0;
}

std::pair<double, double> FrequencyController::getMinMaxFreqFrom(const std::string& attribute) {
    double minFreq = DBL_MAX;
    double maxFreq = -DBL_MAX;
    
    for (int i = 0; ; i++) {
        std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(i) + 
                          "/cpufreq/" + attribute;
        auto value = SysfsReader::readNumber(path);
        if (!value.has_value()) break;
        
        double freqHz = value.value() * 1000.0; // Convert kHz to Hz
        minFreq = std::fmin(minFreq, freqHz);
        maxFreq = std::fmax(maxFreq, freqHz);
    }
    
    if (minFreq == DBL_MAX || maxFreq == -DBL_MAX) {
        return std::make_pair(-1.0, -1.0);
    }
    
    return std::make_pair(minFreq, maxFreq);
}

double FrequencyController::getMinFreq() {
    auto result = getMinMaxFreqFrom("cpuinfo_min_freq");
    return result.first; // Return min
}

double FrequencyController::getMaxFreq() {
    auto result = getMinMaxFreqFrom("cpuinfo_max_freq");
    return result.second; // Return max
}

std::pair<double, double> FrequencyController::getMinMaxFreq() {
    // Get min from cpuinfo_min_freq and max from cpuinfo_max_freq
    double minFreq = getMinFreq();
    double maxFreq = getMaxFreq();
    
    if (minFreq < 0 || maxFreq < 0) {
        return std::make_pair(-1.0, -1.0);
    }
    
    return std::make_pair(minFreq, maxFreq);
}

double FrequencyController::getMaxScalingFreq() {
    auto result = getMinMaxFreqFrom("scaling_max_freq");
    return result.second; // Return max
}

double FrequencyController::getCurrentFreq() {
    auto result = getMinMaxFreqFrom("scaling_cur_freq");
    return result.second; // Return max (all cores should be similar)
}

int FrequencyController::setMaxFrequency(double frequencyHz) {
    // Convert Hz to kHz (sysfs expects kHz)
    int64_t frequencyKHz = static_cast<int64_t>(frequencyHz / 1000.0);
    
    int coresSet = 0;
    for (int i = 0; ; i++) {
        std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(i) + 
                          "/cpufreq/scaling_max_freq";
        
        if (!SysfsReader::writeNumber(path, frequencyKHz)) {
            // If we can't write to this core, we've reached the end
            break;
        }
        coresSet++;
    }
    
    return coresSet > 0 ? coresSet : -1;
}

bool FrequencyController::validateControl() {
    auto minMax = getMinMaxFreq();
    if (minMax.first < 0 || minMax.second < 0) {
        return false;
    }
    
    // Try setting to minimum
    int cores = setMaxFrequency(minMax.first);
    if (cores <= 0) {
        return false;
    }
    
    // Wait a bit for the change to take effect
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check if it was set correctly (allow some tolerance)
    double currentMax = getMaxScalingFreq();
    if (std::abs(currentMax - minMax.first) > 1000.0) { // 1 MHz tolerance
        std::cerr << "Could not set scaling_max_freq to the min frequency." << std::endl;
        std::cerr << "Min freq         : " << (minMax.first / 1000000.0) << " MHz" << std::endl;
        std::cerr << "scaling_max_freq : " << (currentMax / 1000000.0) << " MHz" << std::endl;
        return false;
    }
    
    // Try setting to maximum
    cores = setMaxFrequency(minMax.second);
    if (cores <= 0) {
        return false;
    }
    
    // Wait a bit for the change to take effect
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check if it was set correctly
    currentMax = getMaxScalingFreq();
    if (std::abs(currentMax - minMax.second) > 1000.0) { // 1 MHz tolerance
        std::cerr << "Could not set scaling_max_freq to the max frequency." << std::endl;
        std::cerr << "Max freq         : " << (minMax.second / 1000000.0) << " MHz" << std::endl;
        std::cerr << "scaling_max_freq : " << (currentMax / 1000000.0) << " MHz" << std::endl;
        return false;
    }
    
    return true;
}
