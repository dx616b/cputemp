#ifndef TEMPERATURE_SENSOR_HPP
#define TEMPERATURE_SENSOR_HPP

#include <string>
#include <optional>

class TemperatureSensor {
public:
    // Find and initialize sensor by ID
    // Returns true on success, false on error
    static bool findSensor(const std::string& sensorID, TemperatureSensor& sensor);
    
    // Get current temperature in Celsius
    // Returns empty optional on error
    std::optional<double> readTemperature() const;
    
    // Get the sensor file path
    const std::string& getPath() const { return sensorPath_; }
    
    // Check if sensor is valid
    bool isValid() const { return !sensorPath_.empty(); }
    
    // Default constructor (creates invalid sensor)
    TemperatureSensor() : sensorPath_("") {}
    
private:
    std::string sensorPath_;
    
    TemperatureSensor(const std::string& path) : sensorPath_(path) {}
    
    // Search thermal zones
    static std::optional<std::string> findInThermalZones(const std::string& sensorID);
    
    // Search hwmon devices
    static std::optional<std::string> findInHwmon(const std::string& sensorID);
};

#endif // TEMPERATURE_SENSOR_HPP
