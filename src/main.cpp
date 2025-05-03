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
    Serial.begin(9600);
    delay(5000);

    cfg::begin();

    bleConfig.begin();

    // Initialize I2C
    Wire.begin(8, 9);

    Serial.println("Starting Hermes LED shoes...");

    if (cfg::WAIT_FOR_KEYBOARD) {
        // Wait for serial to initalize.
        while (!Serial) {}

        Serial.println("Strike any key to start...");

        // Wait for the next keystroke.
        while (!Serial.available()) {}

        // Clear the serial buffer.
        Serial.read();
    }

    leds.begin();

    // pull initial config through ConfigManager and subscribe for live updates
    {
        uint16_t initSpeed = ConfigManager::instance().getUint("crawlSpeedMs");
        Serial.printf("initCrawlSpeed %u\n", initSpeed);
        leds.setCrawlSpeed(initSpeed);

        bool initReverse = ConfigManager::instance().getBool("reverseStrip");
        Serial.printf("initReverseStrip %d\n", initReverse);
        leds.setReverseStrip(initReverse);

        ConfigManager::instance().onChangeUint("crawlSpeedMs",
            [&](uint16_t ms){ leds.setCrawlSpeed(ms); });
        ConfigManager::instance().onChangeBool("reverseStrip",
            [&](bool rev){ leds.setReverseStrip(rev); });
    }

    if (!accelSensor.begin()) {
        Serial.println("Accel init failed");
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
