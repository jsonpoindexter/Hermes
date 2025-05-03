#include "ConfigManager.h"

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager() {
    // Seed from EEPROM-backed cfg on startup
    _uintParams["crawlSpeedMs"] = cfg::getCrawlSpeedMs();
    _boolParams["reverseStrip"] = cfg::getReverseStrip();
}

uint16_t ConfigManager::getUint(const std::string& key) const {
    auto it = _uintParams.find(key);
    return (it != _uintParams.end()) ? it->second : 0;
}

bool ConfigManager::getBool(const std::string& key) const {
    auto it = _boolParams.find(key);
    return (it != _boolParams.end()) ? it->second : false;
}

void ConfigManager::onChangeUint(const std::string& key, std::function<void(uint16_t)> cb) {
    _uListeners[key].push_back(cb);
}

void ConfigManager::onChangeBool(const std::string& key, std::function<void(bool)> cb) {
    _bListeners[key].push_back(cb);
}

void ConfigManager::notifyChangeUint(const std::string& key, uint16_t newValue) {
    _uintParams[key] = newValue;
    for (auto& cb : _uListeners[key]) cb(newValue);
}

void ConfigManager::notifyChangeBool(const std::string& key, bool newValue) {
    _boolParams[key] = newValue;
    for (auto& cb : _bListeners[key]) cb(newValue);
}