#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "Config.h"

class ConfigManager {
public:
    static ConfigManager& instance();

    // Read current values
    uint16_t getUint(const std::string& key) const;
    bool     getBool(const std::string& key) const;

    // Subscribe to changes
    void onChangeUint(const std::string& key, std::function<void(uint16_t)> cb);
    void onChangeBool(const std::string& key, std::function<void(bool)> cb);

    // Called by your BLE/EEPROM setters
    void notifyChangeUint(const std::string& key, uint16_t newValue);
    void notifyChangeBool(const std::string& key, bool newValue);

private:
    ConfigManager();
    std::unordered_map<std::string,uint16_t>                     _uintParams;
    std::unordered_map<std::string,bool>                         _boolParams;
    std::unordered_map<std::string,std::vector<std::function<void(uint16_t)>>> _uListeners;
    std::unordered_map<std::string,std::vector<std::function<void(bool)>>>     _bListeners;
};