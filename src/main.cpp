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

// Accel imports.
#include <Adafruit_LSM303_Old.h>

// Our custom data type.
#include "../include/AccelReading.h"

// forward declarations
void loopDebug();
void checkSuperfastHack();
void pauseOnKeystroke();
void accelSetup();
void accelPoll();
bool fillBuffer();
double getVector(AccelReading reading);
bool sleep();
float currentAccelScale();
int bufferSize();
void calibrate();
double getMagnitude(AccelReading reading);
AccelReading getCurrentReading();
AccelReading getPreviousReading();
int getDelta();
void printDelta();
void printMagnitude();
void printBuffer();
bool equalReadings(AccelReading a, AccelReading b);
double calibration; // Baseline for accelerometer data.

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

    accelSetup();
}

void loop() {
    loopDebug();

    accelPoll();
    float scale = currentAccelScale();

    bool isSleeping = sleep();           // returns true/false, no LED work
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

void checkSuperfastHack() {
#if SUPERFAST_LED_HACK
#ifdef _COMPILE_TIME_LEDS_
    Serial.println("Using superfast LED hack.");
#elif
    // Wait for serial to initalize.
    while (!Serial) { }
    Serial.println("WARNING: You need to install the LPD8806Fast library.");
#endif
#endif
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

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

///////////
// accel //
///////////

Adafruit_LSM303_Old lsm; // Bridge to accelerometer hardware.
//Adafruit_LSM303 lsm; // Bridge to accelerometer hardware.
AccelReading accelBuffer[10]; // Buffer for storing the last 10 readings.
int bufferPosition; // Current read position of the buffer.


unsigned long calibrationLEDTime;
bool calibrationLEDOn;

// For breathing, track the time of the last significant movement.
unsigned long lastSignificantMovementTime;

// Initialization.
void accelSetup() {
    if (cfg::WAIT_FOR_KEYBOARD) {
        Serial.println("BEGIN");
    }

    lsm.begin();

    bufferPosition = 0;

    // Initialize the full buffer to zero.
    for (int i = 0; i <bufferSize(); i++) {
        accelBuffer[i].x = 0;
        accelBuffer[i].y = 0;
        accelBuffer[i].z = 0;
    }

    calibrate();
}

void calibrate() {
    if (cfg::WAIT_FOR_KEYBOARD) {
        Serial.println("Calibrating");
    }

    calibration = 0;
    calibrationLEDTime = 0;
    calibrationLEDOn = false;

    leds.showCalibrationPattern();

    while (1) {
        if (cfg::WAIT_FOR_KEYBOARD) {
            Serial.print("...");
        }
        // Update onboard LED.
        unsigned long now = millis();
        if (now - calibrationLEDTime > 250) {
            calibrationLEDTime = now;
            calibrationLEDOn = !calibrationLEDOn;
//            digitalWrite(cfg::ONBOARD_LED_PIN, calibrationLEDOn ? HIGH : LOW);
        }

        // Fill the buffer.
        if (!fillBuffer()) {
            delay(10);
            if (cfg::WAIT_FOR_KEYBOARD) {
                Serial.println("Waiting to fill buffer");
            }
            continue;
        }

        // Check to see if we're done.
        bool pass = true;
        double avg = 0;
        for (int i = 0; i < bufferSize(); i++) {
            double m = getMagnitude(accelBuffer[i]);
            pass = pass && (abs(m - calibration) < 10);
            avg += m;
        }

        if (pass) {
            if (cfg::WAIT_FOR_KEYBOARD) {
                Serial.print("Calibration: ");
                Serial.println(calibration);
            }
            break;
        } else {
            avg /= bufferSize();
            calibration = avg;
            if (cfg::WAIT_FOR_KEYBOARD) {
                Serial.print("Recalculating with average: ");
                Serial.println(calibration);
            }
        }
    }
}

// Gathers data from accelerometer into the buffer. Only writes to the buffer
// if the hardware has gathered data since we last wrote to the buffer.
void accelPoll() {
    // Read new accelerometer data. If there is no new data, return immediately.
    if (!fillBuffer()) {
        return;
    }

    /* PRINT DATA: */
    // printBuffer();
    // printDelta();
    // printMagnitude();
    // Serial.println();
}

// Gets the vector for the given reading.
double getVector(AccelReading reading) {
    double normalizedVector = abs(calibration - getMagnitude(reading));
    return normalizedVector;
}

///////////////////////////////////////////////////////////////////

// This may or may not fill the next buffer position. If the accelerometer hasn't
// processed a new reading since the last buffer, this function immediately exits,
// returning false.
// Otherwise, if the accelerometer has read new data, this function advances the
// buffer position, fills the buffer with accelerometer data, and returns true.
bool fillBuffer() {
    // Read from the hardware.
    lsm.read();

    AccelReading newReading;
    newReading.x = lsm.accelData.x;
    newReading.y = lsm.accelData.y;
    newReading.z = lsm.accelData.z;

    // The accelerometer hasn't processed a new reading since the last buffer.
    // Do nothing and return false.
    if (equalReadings(getCurrentReading(), newReading)) {
        return false;
    }

    // The accelerometer has read new data.

    // Advance the buffer.
    if (++bufferPosition >= bufferSize()) {
        bufferPosition = 0;
    }

    AccelReading *mutableCurrentReading = &accelBuffer[bufferPosition];

    mutableCurrentReading->x = newReading.x;
    mutableCurrentReading->y = newReading.y;
    mutableCurrentReading->z = newReading.z;

    return true;
}

///////////////////////////////////////////////////////////////////

// Gets the average difference between the latest buffer and previous buffer.
int getDelta() {
    AccelReading previousReading = getPreviousReading();
    AccelReading currentReading = getCurrentReading();

    int deltaX = abs(abs(currentReading.x) - abs(previousReading.x));
    int deltaY = abs(abs(currentReading.y) - abs(previousReading.y));
    int deltaZ = abs(abs(currentReading.z) - abs(previousReading.z));

    return (deltaX + deltaY + deltaZ) / 3;
}

void printDelta() {
    AccelReading previousReading = getPreviousReading();
    AccelReading currentReading = getCurrentReading();

    int deltaX = abs(abs(currentReading.x) - abs(previousReading.x));
    int deltaY = abs(abs(currentReading.y) - abs(previousReading.y));
    int deltaZ = abs(abs(currentReading.z) - abs(previousReading.z));

    Serial.print(deltaX);
    Serial.print("\t");
    Serial.print(deltaY);
    Serial.print("\t");
    Serial.print(deltaZ);
    Serial.print("\t");
    Serial.print(getDelta());
    Serial.println();
}

// Gets the vector magnitude for the given reading.
// http://en.wikipedia.org/wiki/Euclidean_vector#Length
double getMagnitude(AccelReading reading) {
    double x = reading.x;
    double y = reading.y;
    double z = reading.z;

    double vector = x * x + y * y + z * z;

    return sqrt(vector);
}

void printMagnitude() {
    Serial.println(getMagnitude(getCurrentReading()));
}

// Prints the latest buffer reading to the screen.
void printBuffer() {
    Serial.print(accelBuffer[bufferPosition].x);
    Serial.print("\t");
    Serial.print(accelBuffer[bufferPosition].y);
    Serial.print("\t");
    Serial.print(accelBuffer[bufferPosition].z);
    Serial.println();
}

///////////////////////////////////////////////////////////////////

// Returns the number of items held by the buffer.
int bufferSize() {
    return sizeof(accelBuffer) / sizeof(accelBuffer[0]);
}

AccelReading getCurrentReading() {
    return accelBuffer[bufferPosition];
}

// Gets the previous buffer reading.
AccelReading getPreviousReading() {
    int previous = bufferPosition - 1;
    if (previous < 0) previous = bufferSize() - 1;
    return accelBuffer[previous];
}

// Returns true if two readings are equal.
bool equalReadings(AccelReading a, AccelReading b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

// TODO: move this into an AccelSensor class once we extract accel logic
// Returns 0‒1 scale of current movement relative to sensitivity
float currentAccelScale() {
    double vec = abs(calibration - getMagnitude(getCurrentReading()));
    return vec / cfg::HERMES_SENSITIVITY;
}

///////////
// sleep //
///////////
bool sleep()
{
    unsigned long now = millis();

    // If movement exceeds threshold, update the timer
    double m = getMagnitude(getCurrentReading());
    if (abs(calibration - m) > cfg::SLEEP_SENSITIVITY)
        lastSignificantMovementTime = now;

    // Decide sleeping state purely on timeout
    return (now - lastSignificantMovementTime) >= cfg::SLEEP_WAIT_TIME_MS;
}

///////////////////////////////////////////////////////////////////
