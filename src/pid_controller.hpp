#ifndef PID_CONTROLLER_HPP
#define PID_CONTROLLER_HPP

class PIDController {
public:
    // Constructor with configurable gains
    // C1: proportional gain, C2: derivative gain
    PIDController(double C1, double C2, double minFreq, double maxFreq);
    
    // Calculate next frequency based on current and previous temperature
    // Returns frequency in Hz, clamped to min/max bounds
    double calculate(double currentTemp, double targetTemp, double previousTemp);
    
    // Get current frequency state
    double getCurrentFreq() const { return currentFreq_; }
    
    // Reset controller state
    void reset(double initialFreq);
    
private:
    double C1_;  // Proportional gain
    double C2_;  // Derivative gain
    double minFreq_;
    double maxFreq_;
    double currentFreq_;
};

#endif // PID_CONTROLLER_HPP
