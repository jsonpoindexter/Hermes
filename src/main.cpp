/*
 * Hermes LED shoes
 * Copyright 2013-2014 RGAM LLC
 *
 */
///////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "ConfigManager.h"
#include "debug.h"
#include "BLEConfigService.h"
#include "AccelSensor.h"

#include <LedStrip.h>

static BLEConfigService bleConfig;
static LedStrip leds;
static AccelSensor accelSensor;


void setup() {
#ifdef DEBUG
    Serial.begin(9600);
    delay(5000);
#endif

    cfg::begin();

    bleConfig.begin();

    DEBUG_PRINTLN("Starting Hermes LED shoes...");

    if (cfg::WAIT_FOR_KEYBOARD) {
        // Wait for serial to initalize.
        while (!Serial) {}

        DEBUG_PRINTLN("Strike any key to start...");

        // Wait for the next keystroke.
        while (!Serial.available()) {}

        // Clear the serial buffer.
        Serial.read();
    }

    leds.begin();
    leds.showCalibrationPattern();

    // pull initial config through ConfigManager and subscribe for live updates
    {
        uint16_t initSpeed = ConfigManager::instance().getUint("crawlSpeedMs");
        DEBUG_PRINTF("initCrawlSpeed %u\n", initSpeed);
        leds.setCrawlSpeed(initSpeed);

        bool initReverse = ConfigManager::instance().getBool("reverseStrip");
        DEBUG_PRINTF("initReverseStrip %d\n", initReverse);
        leds.setReverseStrip(initReverse);

        ConfigManager::instance().onChangeUint("crawlSpeedMs",
                                               [&](uint16_t ms) { leds.setCrawlSpeed(ms); });
        ConfigManager::instance().onChangeBool("reverseStrip",
                                               [&](bool rev) { leds.setReverseStrip(rev); });
    }

    // Initialize I2C
    Wire.begin(8, 9);
    if (!accelSensor.begin()) {
        DEBUG_PRINTF("Accel init failed");
    }
}

void loop() {
    loopDebug();

    accelSensor.poll();
    float scale = accelSensor.currentAccelScale();
    bool isSleeping = accelSensor.isSleeping();
    // returns true/false, no LED work
    leds.update(scale, isSleeping);
}
