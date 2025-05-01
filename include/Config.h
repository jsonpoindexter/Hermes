#pragma once

namespace cfg {
// --------- Hardware pins ----------
    constexpr uint8_t DATA_PIN         = 6;
    constexpr uint8_t ONBOARD_LED_PIN  = 7;

// --------- LED strip --------------
    constexpr uint16_t LED_COUNT       = 44;
    constexpr bool     REVERSE_STRIP   = true;

// --------- Animation --------------
    constexpr uint16_t CRAWL_SPEED_MS  = 35;
    constexpr float    HERMES_SENSITIVITY = 1600.0f;
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

} // namespace cfg

