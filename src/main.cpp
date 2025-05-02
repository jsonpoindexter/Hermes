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

#include <LedStrip.h>
static LedStrip leds;

#include "AccelSensor.h"
static AccelSensor accelSensor;

void setup() {
    Serial.begin(9600);

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

    if (!accelSensor.begin()) {
        Serial.println("Accel init failed");
    }
}

void loop() {
    loopDebug();

    accelSensor.poll();
    float scale = accelSensor.currentAccelScale();

    bool isSleeping = accelSensor.isSleeping();           // returns true/false, no LED work
    leds.update(scale, isSleeping);
}
