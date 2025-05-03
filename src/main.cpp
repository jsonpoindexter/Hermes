/*
 * Hermes LED shoes
 * Copyright 2013-2014 RGAM LLC
 *
 */
///////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
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
    Serial.println("BLE Config ready");

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

    // initialize crawl speed from persisted config
    leds.setCrawlSpeed(cfg::getCrawlSpeedMs());

    if (!accelSensor.begin()) {
        Serial.println("Accel init failed");
    }
}

void loop() {
    loopDebug();

    // handle BLE callbacks
    bleConfig.poll();
    // update crawl speed in case it changed via BLE
    leds.setCrawlSpeed(cfg::getCrawlSpeedMs());

    accelSensor.poll();
    float scale = accelSensor.currentAccelScale();
    bool isSleeping = accelSensor.isSleeping();
    // returns true/false, no LED work
    leds.update(scale, isSleeping);
}
