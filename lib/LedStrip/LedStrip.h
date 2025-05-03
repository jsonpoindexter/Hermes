#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <../../include/Config.h>

class LedStrip {
public:
    void begin();
    void update(float accelScale, bool isSleeping);

    // configure crawl speed at runtime
    void setCrawlSpeed(uint16_t ms);
    // configure reverse strip at runtime
    void setReverseStrip(bool reverse);

    void showCalibrationPattern();          // simple flash during accel calibration

    void colorOff();

private:
    /* ---------- helpers ---------- */
    void crawl(uint32_t color);
    void breathe();
    void showSolid(float scale);
    uint32_t colorForScale(float scale) const;
    uint32_t colorWheel(uint16_t colorIdx, float brightness) const;

    inline int  mapIndex(int logical) const;
    inline int  constrainWrap(int v, int low, int high) const;
    void        stripShow();

    /* ---------- state ---------- */
    static constexpr uint16_t COLOR_RANGE = 384;

    Adafruit_NeoPixel strip{cfg::LED_COUNT, cfg::DATA_PIN, NEO_GRB + NEO_KHZ800};
    uint32_t  lightArray[cfg::LED_COUNT]{};
    uint32_t  lastColor   = 0;
    unsigned long lastCrawl  = 0;
    unsigned long lastBreath = 0;
    int  keyframePtr = 0;
    bool sleeping    = false;
    // runtime-controlled crawl speed
    uint16_t crawlSpeedMs = cfg::DEFAULT_CRAWL_SPEED_MS;
    // runtime-controlled reverse strip
    bool reverseStrip = cfg::DEFAULT_REVERSE_STRIP;


};