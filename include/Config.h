#include <cstdint>
#pragma once

namespace cfg {
// --------- Hardware pins ----------
    constexpr uint8_t DATA_PIN         = 10;

// --------- LED strip --------------
    constexpr uint16_t LED_COUNT       = 44;
    constexpr bool     DEFAULT_REVERSE_STRIP   = true;

// --------- Animation --------------
    constexpr float    HERMES_SENSITIVITY = 1600.0;
    constexpr bool     ENABLE_SPLIT_STRIP = false;
    constexpr uint16_t SPLIT_STRIP_CENTER = 0;

// --------- Sleep ------------------
    constexpr uint32_t SLEEP_CYCLE_MS     = 5000;
    constexpr uint32_t SLEEP_WAIT_TIME_MS = 5000;
    constexpr uint8_t  SLEEP_SENSITIVITY  = 25;

// --------- Brightness ------------
    constexpr float    MAX_BRIGHTNESS      = 1.0f;
    constexpr float    MIN_BRIGHTNESS      = 1.0f;
    constexpr float    SLEEP_BRIGHTNESS    = 0.30f;

// --------- Debug -----------------
    constexpr bool     WAIT_FOR_KEYBOARD   = false;
    constexpr bool     PRINT_LOOP_TIME     = false;
    // your _default_ compile-time constant
    constexpr uint16_t DEFAULT_CRAWL_SPEED_MS = 2;

    // the _runtime_ value (goes in RAM)
    extern uint16_t crawlSpeedMs;

    // reverse strip
    extern bool reverseStrip;

    // call once in setup()
    void      begin();

    // getters/setters
    uint16_t  getCrawlSpeedMs();
    void      setCrawlSpeedMs(uint16_t ms);
    bool      getReverseStrip();
    void      setReverseStrip(bool reverse);

} // namespace cfg

