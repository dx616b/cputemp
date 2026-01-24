#include "temperature_sensor.hpp"
#include "sysfs_reader.hpp"
#include <iostream>
#include <cstdlib>

std::optional<std::string> TemperatureSensor::findInThermalZones(const std::string& sensorID) {
    for (int i = 0; ; i++) {
        std::string typePath = "/sys/class/thermal/thermal_zone" + std::to_string(i) + "/type";
        auto type = SysfsReader::readString(typePath);
        if (!type.has_value()) break;
        
        if (type.value() != sensorID) continue;
        
        std::string tempPath = "/sys/class/thermal/thermal_zone" + std::to_string(i) + "/temp";
        auto temp = SysfsReader::readNumber(tempPath);
        if (temp.has_value()) {
            return tempPath;
        }
    }
    return std::nullopt;
}

std::optional<std::string> TemperatureSensor::findInHwmon(const std::string& sensorID) {
    for (int i = 0; ; i++) {
        std::string namePath = "/sys/class/hwmon/hwmon" + std::to_string(i) + "/name";
        auto name = SysfsReader::readString(namePath);
        if (!name.has_value()) break;
        
        if (name.value() != sensorID) continue;
        
        std::string tempPath = "/sys/class/hwmon/hwmon" + std::to_string(i) + "/temp1_input";
        auto temp = SysfsReader::readNumber(tempPath);
        if (temp.has_value()) {
            return tempPath;
        }
    }
    return std::nullopt;
}

bool TemperatureSensor::findSensor(const std::string& sensorID, TemperatureSensor& sensor) {
    // Try thermal zones first
    auto path = findInThermalZones(sensorID);
    if (path.has_value()) {
        sensor = TemperatureSensor(path.value());
        return true;
    }
    
    // Try hwmon devices
    path = findInHwmon(sensorID);
    if (path.has_value()) {
        sensor = TemperatureSensor(path.value());
        return true;
    }
    
    std::cerr << "Could not find sensor " << sensorID << std::endl;
    return false;
}

std::optional<double> TemperatureSensor::readTemperature() const {
    if (sensorPath_.empty()) {
        return std::nullopt;
    }
    
    auto temp = SysfsReader::readNumber(sensorPath_);
    if (temp.has_value()) {
        // Convert from millidegrees to degrees Celsius
        return temp.value() * 0.001;
    }
    
    return std::nullopt;
}
