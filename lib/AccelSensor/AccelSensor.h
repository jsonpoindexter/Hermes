#pragma once

#include <Arduino.h>
#include <Adafruit_LSM303_Old.h>
#include "../../include/AccelReading.h"
#include "../../include/Config.h"

class AccelSensor {
public:
    AccelSensor();

    bool begin();

    void poll();

    double currentAccelScale() const;

    bool isSleeping() const;

    void setSensitivity(uint16_t sensitivity);

    void calibrate();

    uint8_t currentScale8() const;

private:

    bool fillBuffer();

    static double getMagnitude(const AccelReading &r);

    static bool equalReadings(const AccelReading &a, const AccelReading &b);

    int bufferSize() const;

    const AccelReading &getCurrentReading() const;

    const AccelReading &getPreviousReading() const;

    Adafruit_LSM303_Old lsm;
    AccelReading accelBuffer[10]{};
    int bufferPosition;
    unsigned long calibrationLEDTime;
    bool calibrationLEDOn;
    unsigned long lastSignificantMovementTime;
    double calibration;

    uint16_t hermesSensitivity = cfg::DEFAULT_HERMES_SENSITIVITY;

};
