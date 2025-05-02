/*
 * Hermes LED shoes
 * Copyright 2013-2014 RGAM LLC
 *
 */
///////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"

#include <LedStrip.h>
static LedStrip leds;

#include "AccelSensor.h"
static AccelSensor accelSensor;

// forward declarations
void loopDebug();
void checkSuperfastHack();
void pauseOnKeystroke();

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

// Debug functions controlled by run/debug parameters.
unsigned long before = 0;

void loopDebug() {
    if (cfg::WAIT_FOR_KEYBOARD) {
        pauseOnKeystroke();
    }
    if (cfg::PRINT_LOOP_TIME) {
        unsigned long now = millis();
        Serial.println(now - before);
        before = millis();
    }
}

void pauseOnKeystroke() {
    if (Serial.available()) {
        // Clear the serial buffer.
        Serial.read();

        Serial.println("Paused. Strike any key to resume...");

        // Turn all LEDs off.
        leds.colorOff();

        // Wait for the next keystroke.
        while (!Serial.available()) {}

        // Clear the serial buffer.
        Serial.read();
    }
}