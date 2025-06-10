#include <cstdint>

#pragma once

// Debug prints control
// Set to 1 to enable debug prints, 0 to disable
#define CONFIG_DEBUG 0

#include <vector>
#include <memory>
#include "ConfigParameter.h"

namespace cfg {
// --------- Hardware pins ----------
    constexpr uint8_t DATA_PIN = 10;

// --------- LED strip --------------
    constexpr uint16_t LED_COUNT = 44;
    constexpr bool DEFAULT_REVERSE_STRIP = true;

// --------- Animation --------------
    constexpr uint16_t DEFAULT_HERMES_SENSITIVITY = 1600; // lower is more sensitive
    constexpr bool ENABLE_SPLIT_STRIP = false;
    constexpr uint16_t SPLIT_STRIP_CENTER = 0;

// --------- Sleep ------------------
    constexpr uint32_t SLEEP_CYCLE_MS = 5000;
    constexpr uint32_t SLEEP_WAIT_TIME_MS = 5000;
    constexpr uint8_t SLEEP_SENSITIVITY = 25;

// --------- Brightness ------------
    constexpr uint8_t MAX_BRIGHTNESS = 255;
    constexpr uint8_t MIN_BRIGHTNESS = 255;
    constexpr uint8_t SLEEP_BRIGHTNESS = 64;

// --------- Color -----------------
    constexpr uint8_t DEFAULT_BASE_HUE = 0;     // 0‑255 hue offset for the wheel

// --------- Debug -----------------
    constexpr bool WAIT_FOR_KEYBOARD = false;
    constexpr bool PRINT_LOOP_TIME = false;
    constexpr uint16_t DEFAULT_CRAWL_SPEED_MS = 2;

    // call once in setup()
    void begin();

    // runtime accessors (used by ConfigManager and others)
    uint16_t getCrawlSpeedMs();

    bool getReverseStrip();

    uint16_t getHermesSensitivity();

    uint8_t getBaseHue();

    // register config parameters
    void registerParameters(std::vector<std::unique_ptr<IConfigParameter>> &list);

} // namespace cfg
