#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <Config.h>

class LedStrip {
public:
    void begin();

    void update(uint8_t scale8, bool isSleeping);

    // configure crawl speed at runtime
    void setCrawlSpeed(uint16_t ms);

    // configure reverse strip at runtime
    void setReverseStrip(bool reverse);

    void showCalibrationPattern();          // simple flash during accel calibration

    static void colorOff();

    bool showingCalibrationPattern;
private:
    /* ---------- helpers ---------- */
    void crawl(const CRGB &color);

    void breathe();

    void showSolid(uint8_t scale8);

    static CRGB colorForScale(uint8_t scale8);

    /* ---------- state ---------- */
    static constexpr uint16_t COLOR_RANGE = 256;   // 0‑255 hue wheel

    CRGB ledsArr[cfg::LED_COUNT];
    CRGB lightArray[cfg::LED_COUNT]{};
    CRGB lastColor = 0;
    unsigned long lastCrawl = 0;
    unsigned long lastBreath = 0;
    int keyframePtr = 0;
    bool sleeping = false;
    // runtime-controlled crawl speed
    uint16_t crawlSpeedMs = cfg::DEFAULT_CRAWL_SPEED_MS;
    // runtime-controlled reverse strip
    bool reverseStrip = cfg::DEFAULT_REVERSE_STRIP;


    static CRGB wheel[COLOR_RANGE];
    static int physicalIndex[cfg::LED_COUNT];

    void buildIndexMap() const;

};