#include "AccelSensor.h"
#include <Wire.h>
#include <math.h>

AccelSensor::AccelSensor()
  : lsm(), bufferPosition(0),
    calibrationLEDTime(0), calibrationLEDOn(false),
    lastSignificantMovementTime(0), calibration(0.0) {}

bool AccelSensor::begin() {
    Serial.println("BEGIN");
    Wire.begin();
    if (!lsm.begin()) {
        return false;
    }
    for (int i = 0; i < bufferSize(); ++i) {
        accelBuffer[i] = {0, 0, 0};
    }
    calibrate();
    return true;
}

void AccelSensor::calibrate() {
    Serial.println("Calibrating");
    calibration = 0;
    calibrationLEDTime = 0;
    calibrationLEDOn = false;
    while (true) {
        if (!fillBuffer()) {
            delay(10);
            continue;
        }
        bool pass = true;
        double sum = 0;
        for (int i = 0; i < bufferSize(); ++i) {
            double m = getMagnitude(accelBuffer[i]);
            pass &= (abs(m - calibration) < 10);
            sum += m;
        }
        if (pass) break;
        calibration = sum / bufferSize();
    }
}

void AccelSensor::poll() {
    fillBuffer();
}

bool AccelSensor::fillBuffer() {
    lsm.read();
    AccelReading newR{lsm.accelData.x, lsm.accelData.y, lsm.accelData.z};
    if (equalReadings(getCurrentReading(), newR)) {
        return false;
    }
    bufferPosition = (bufferPosition + 1) % bufferSize();
    accelBuffer[bufferPosition] = newR;
    if (abs(getMagnitude(newR) - calibration) > cfg::SLEEP_SENSITIVITY) {
        lastSignificantMovementTime = millis();
    }
    return true;
}

double AccelSensor::getMagnitude(const AccelReading& r) const {
    return sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
}

bool AccelSensor::equalReadings(const AccelReading& a, const AccelReading& b) const {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

int AccelSensor::bufferSize() const {
    return sizeof(accelBuffer) / sizeof(accelBuffer[0]);
}

const AccelReading& AccelSensor::getCurrentReading() const {
    return accelBuffer[bufferPosition];
}

const AccelReading& AccelSensor::getPreviousReading() const {
    int prev = bufferPosition ? bufferPosition - 1 : bufferSize() - 1;
    return accelBuffer[prev];
}

float AccelSensor::currentAccelScale() const {
    double delta = abs(getMagnitude(getCurrentReading()) - calibration);
    return delta / cfg::HERMES_SENSITIVITY;
}

bool AccelSensor::isSleeping() const {
    return (millis() - lastSignificantMovementTime) >= cfg::SLEEP_WAIT_TIME_MS;
}
