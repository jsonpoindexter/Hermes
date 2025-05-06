#include <../../include/Config.h> // ensure DEFAULT_CRAWL_SPEED_MS is available
#include <debug.h>
#include "LedStrip.h"
#include <FastLED.h>

// Circular buffer head index for crawl animation

void LedStrip::setCrawlSpeed(uint16_t ms) {
    crawlSpeedMs = ms;
}

void LedStrip::setReverseStrip(bool reverse) {
    reverseStrip = reverse;
}

void LedStrip::begin() {
    FastLED.addLeds<NEOPIXEL, cfg::DATA_PIN>(ledsArr, cfg::LED_COUNT);
    FastLED.clear();
    FastLED.show();
}

void LedStrip::update(float accelScale, bool isSleeping) {
    sleeping = isSleeping;
    if (sleeping) {
        breathe();
        return;
    }
    uint32_t c = colorForScale(accelScale);
    crawl(c);
}

/* -------------------------------------------------- */
uint32_t LedStrip::colorWheel(uint16_t color, float brightness) const {
    color = constrain(color, 0, COLOR_RANGE - 1);     // 0-383
    uint8_t r = 0, g = 0, b = 0;

    switch (color / 128) {             // 0–127, 128–255, 256–383
        case 0:
            r = 127 - color % 128;
            g = color % 128;
            break; // red→yellow
        case 1:
            g = 127 - color % 128;
            b = color % 128;
            break; // yellow→teal
        case 2:
            r = color % 128;
            b = 127 - color % 128;
            break; // teal→purple
    }
    r *= brightness;
    g *= brightness;
    b *= brightness;
    return (r << 16) | (g << 8) | b;
}

uint32_t LedStrip::colorForScale(float scale) const {
    scale = constrain(scale, 0.0f, 1.0f);
    float brightness = cfg::MAX_BRIGHTNESS * (scale + cfg::MIN_BRIGHTNESS);
    uint16_t idx = static_cast<uint16_t>(COLOR_RANGE * scale);
    return colorWheel(idx, brightness);
}

inline int LedStrip::constrainWrap(int v, int low, int high) const {
    if (v < low) return high - (low - v) + 1;
    if (v > high) return low + (v - high) - 1;
    return v;
}

inline int LedStrip::mapIndex(int logical) const {
    return reverseStrip ? (cfg::LED_COUNT - 1 - logical) : logical;
}

/* ------------ crawl animation ------------------ */
int head = 0; // Circular buffer head index
void LedStrip::crawl(uint32_t color) {
    // Determine previous head color
    int prevHeadIdx = head;
    uint32_t prevHeadColor = lightArray[prevHeadIdx];

    unsigned long now = millis();
    bool needShift = (now - lastCrawl) > crawlSpeedMs || (color != prevHeadColor);
    if (!needShift) return;

    lastCrawl = now;

    // Advance head backwards (circular)
    head = (head + cfg::LED_COUNT - 1) % cfg::LED_COUNT;
    lightArray[head] = color;

    if (cfg::ENABLE_SPLIT_STRIP) {
        int center = cfg::SPLIT_STRIP_CENTER;
        int perSide = cfg::LED_COUNT / 2;

        for (int i = 0; i < perSide; ++i) {
            uint32_t c = lightArray[(head + i) % cfg::LED_COUNT];
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8) & 0xFF;
            uint8_t b = c & 0xFF;
            ledsArr[mapIndex(center - 1 - i)] = CRGB(r, g, b);
        }
        for (int i = 0; i < perSide; ++i) {
            uint32_t c = lightArray[(head + perSide + i) % cfg::LED_COUNT];
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8) & 0xFF;
            uint8_t b = c & 0xFF;
            ledsArr[mapIndex(center + i)] = CRGB(r, g, b);
        }
    } else {
        for (int i = 0; i < cfg::LED_COUNT; ++i) {
            uint32_t c = lightArray[(head + i) % cfg::LED_COUNT];
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8) & 0xFF;
            uint8_t b = c & 0xFF;
            ledsArr[mapIndex(i)] = CRGB(r, g, b);
        }
    }
    FastLED.show();
}

/* ------------ breathing animation -------------- */
namespace {
    const uint8_t KEYFRAMES[] = {
            20, 21, 22, 24, 26, 28, 31, 34, 38, 41, 45, 50, 55, 60, 66, 73, 80, 87, 95,
            103, 112, 121, 131, 141, 151, 161, 172, 182, 192, 202, 211, 220, 228, 236, 242, 247, 251, 254, 255,
            254, 251, 247, 242, 236, 228, 220, 211, 202, 192, 182, 172, 161, 151, 141, 131, 121, 112, 103, 95,
            87, 80, 73, 66, 60, 55, 50, 45, 41, 38, 34, 31, 28, 26, 24, 22, 21, 20,
            20, 20, 20, 20, 20, 20, 20, 20, 20, 20
    };
}

void LedStrip::breathe() {
    constexpr int frames = sizeof(KEYFRAMES);
    float period = static_cast<float>(cfg::SLEEP_CYCLE_MS) / frames;
    unsigned long now = millis();

    if ((now - lastBreath) <= period) return;
    lastBreath = now;

    uint8_t key = KEYFRAMES[keyframePtr];
    for (int i = 0; i < cfg::LED_COUNT; ++i) {
        uint8_t v = (cfg::SLEEP_BRIGHTNESS * 127 * key) / 256;
        ledsArr[mapIndex(i)] = CRGB(v, 0, 0);
    }
    FastLED.show();

    if (++keyframePtr >= frames) keyframePtr = 0;
}

/* ------------ misc ----------------------------- */
void LedStrip::showSolid(float scale) {
    uint32_t c = colorForScale(scale);
    if (c == lastColor) return;
    lastColor = c;

    for (int i = 0; i < cfg::LED_COUNT; ++i) {
        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8) & 0xFF;
        uint8_t b = c & 0xFF;
        ledsArr[mapIndex(i)] = CRGB(r, g, b);
    }

    FastLED.show();
}

/* ------------ public helpers ------------------ */
void LedStrip::colorOff() {
    FastLED.clear();
    FastLED.show();
}

void LedStrip::showCalibrationPattern() {
    DEBUG_PRINTLN("showCalibrationPattern...");

    LedStrip::colorOff();

    int mid = cfg::LED_COUNT / 2;
    float brightness = 0.3;

    // Red
    ledsArr[mapIndex(mid - 1)] = CRGB(static_cast<uint8_t>(127 * brightness), 0, 0);
    // Green
    ledsArr[mapIndex(mid)] = CRGB(0, static_cast<uint8_t>(127 * brightness), 0);
    // Blue
    ledsArr[mapIndex(mid + 1)] = CRGB(0, 0, static_cast<uint8_t>(127 * brightness));

    FastLED.show();
}