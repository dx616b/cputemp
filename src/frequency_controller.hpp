#ifndef FREQUENCY_CONTROLLER_HPP
#define FREQUENCY_CONTROLLER_HPP

#include <string>
#include <utility>

class FrequencyController {
public:
    // Get minimum available frequency (in Hz)
    // Returns -1 on error
    static double getMinFreq();
    
    // Get maximum available frequency (in Hz)
    // Returns -1 on error
    static double getMaxFreq();
    
    // Get minimum and maximum available frequencies (in Hz)
    // Returns pair<min, max> or pair<-1, -1> on error
    static std::pair<double, double> getMinMaxFreq();
    
    // Get current maximum scaling frequency (in Hz)
    static double getMaxScalingFreq();
    
    // Get current actual frequency (in Hz)
    static double getCurrentFreq();
    
    // Set maximum scaling frequency for all CPU cores (in Hz)
    // Returns number of cores successfully set, or -1 on error
    static int setMaxFrequency(double frequencyHz);
    
    // Validate that frequency control works
    // Returns true if we can set min and max frequencies
    static bool validateControl();
    
private:
    // Read frequency from a specific CPU core's cpufreq file
    static double readCpuFreq(const std::string& filename);
    
    // Get min/max from a specific cpufreq attribute
    static std::pair<double, double> getMinMaxFreqFrom(const std::string& attribute);
};

#endif // FREQUENCY_CONTROLLER_HPP
