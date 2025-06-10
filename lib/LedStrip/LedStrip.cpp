#include <../../include/Config.h>
#include <debug.h>
#include "LedStrip.h"
#include <FastLED.h>

// Definition of static wheel array
CRGB LedStrip::wheel[LedStrip::COLOR_RANGE];

// Definition of static physical index array
int LedStrip::physicalIndex[cfg::LED_COUNT];

void LedStrip::setCrawlSpeed(uint16_t ms) {
    crawlSpeedMs = ms;
}

void LedStrip::setReverseStrip(bool reverse) {
    reverseStrip = reverse;
    LedStrip::buildIndexMap();
}

void LedStrip::begin() {
    CFastLED::addLeds<NEOPIXEL, cfg::DATA_PIN>(ledsArr, cfg::LED_COUNT);
    FastLED.clear();
    FastLED.show();

    // Build 256-step HSV wheel (0-255)
    for (uint16_t i = 0; i < COLOR_RANGE; ++i) {
        wheel[i] = CHSV(i, 255, 255);
    }

    LedStrip::buildIndexMap();
}

void LedStrip::buildIndexMap() const {
    for (uint16_t i = 0; i < cfg::LED_COUNT; ++i) {
        physicalIndex[i] = reverseStrip ? (cfg::LED_COUNT - 1 - i) : i;
    }
};

void LedStrip::update(uint8_t scale8, bool isSleeping) {
    sleeping = isSleeping;
    if (sleeping) {
        breathe();
        return;
    }
    CRGB c = colorForScale(scale8);
    crawl(c);
}

/* ------------ colorForScale ------------------ */
CRGB LedStrip::colorForScale(uint8_t scale8) {
    // Hue 0-255 maps directly
    CRGB c = wheel[scale8];
    // Brightness scaling 0-255 → MIN..MAX (uint8 math)
    uint8_t b8 = cfg::MIN_BRIGHTNESS +
                 (uint16_t(scale8) * (cfg::MAX_BRIGHTNESS - cfg::MIN_BRIGHTNESS)) / 255;
    c.nscale8_video(b8);
    return c;
}

/* ------------ crawl animation ------------------ */
int head = 0; // Circular buffer head index
void LedStrip::crawl(const CRGB &color) {
    if (showingCalibrationPattern) {
        // Don't crawl while showing calibration pattern
        return;
    }
    // Determine previous head color
    int prevHeadIdx = head;
    CRGB prevHeadColor = lightArray[prevHeadIdx];

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
            ledsArr[physicalIndex[center - 1 - i]] = lightArray[(head + i) % cfg::LED_COUNT];
        }
        for (int i = 0; i < perSide; ++i) {
            ledsArr[physicalIndex[center + i]] = lightArray[(head + perSide + i) % cfg::LED_COUNT];
        }
    } else {
        for (int i = 0; i < cfg::LED_COUNT; ++i) {
            ledsArr[physicalIndex[i]] = lightArray[(head + i) % cfg::LED_COUNT];
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
    uint8_t v = (uint16_t(cfg::SLEEP_BRIGHTNESS) * key) / 255;
    fill_solid(ledsArr, cfg::LED_COUNT, CRGB(v, 0, 0));
    FastLED.show();

    if (++keyframePtr >= frames) keyframePtr = 0;
}

/* ------------ misc ----------------------------- */
void LedStrip::showSolid(uint8_t scale8) {
    CRGB c = colorForScale(scale8);
    if (c == lastColor) return;
    lastColor = c;

    fill_solid(ledsArr, cfg::LED_COUNT, c);
    FastLED.show();
}

/* ------------ public helpers ------------------ */
void LedStrip::colorOff() {
    FastLED.clear();
    FastLED.show();
}

void LedStrip::showCalibrationPattern() {
    DEBUG_PRINTLN("showCalibrationPattern...");

    LedStrip::showingCalibrationPattern = true;

    LedStrip::colorOff();

    int mid = cfg::LED_COUNT / 2;
    float brightness = 0.3;

    // Red
    ledsArr[physicalIndex[mid - 1]] = CRGB(static_cast<uint8_t>(127 * brightness), 0, 0);
    // Green
    ledsArr[physicalIndex[mid]] = CRGB(0, static_cast<uint8_t>(127 * brightness), 0);
    // Blue
    ledsArr[physicalIndex[mid + 1]] = CRGB(0, 0, static_cast<uint8_t>(127 * brightness));

    FastLED.show();
}