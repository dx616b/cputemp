#include "pid_controller.hpp"
#include <algorithm>
#include <cmath>

PIDController::PIDController(double C1, double C2, double minFreq, double maxFreq)
    : C1_(C1), C2_(C2), minFreq_(minFreq), maxFreq_(maxFreq), currentFreq_(maxFreq) {
}

double PIDController::calculate(double currentTemp, double targetTemp, double previousTemp) {
    // PID-like control: nextFreq = curFreq + C1*(target - current) - C2*(current - previous)
    // This is actually a PI controller with derivative term on temperature change
    double nextFreq = currentFreq_ + C1_ * (targetTemp - currentTemp) - C2_ * (currentTemp - previousTemp);
    
    // Clamp to bounds
    nextFreq = std::max(minFreq_, std::min(maxFreq_, nextFreq));
    
    currentFreq_ = nextFreq;
    return nextFreq;
}

void PIDController::reset(double initialFreq) {
    currentFreq_ = std::max(minFreq_, std::min(maxFreq_, initialFreq));
}
