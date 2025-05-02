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
    float currentAccelScale() const;
    bool isSleeping() const;

private:
    void calibrate();
    bool fillBuffer();
    double getMagnitude(const AccelReading& r) const;
    bool equalReadings(const AccelReading& a, const AccelReading& b) const;
    int bufferSize() const;
    const AccelReading& getCurrentReading() const;
    const AccelReading& getPreviousReading() const;

    Adafruit_LSM303_Old lsm;
    AccelReading accelBuffer[10];
    int bufferPosition;
    unsigned long calibrationLEDTime;
    bool calibrationLEDOn;
    unsigned long lastSignificantMovementTime;
    double calibration;
};
