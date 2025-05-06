#include "AccelSensor.h"
#include <Wire.h>
#include <math.h>
#include "debug.h"

AccelSensor::AccelSensor()
        : lsm(), bufferPosition(0),
          calibrationLEDTime(0), calibrationLEDOn(false),
          lastSignificantMovementTime(0), calibration(0.0) {}

bool AccelSensor::begin() {
    DEBUG_PRINTLN("BEGIN");
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
    DEBUG_PRINT("Calibrating");
    calibration = 0;
    calibrationLEDTime = 0;
    calibrationLEDOn = false;
    while (true) {
        DEBUG_PRINT("...");
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
    DEBUG_PRINT("Calibration complete: ");
    DEBUG_PRINTLN(calibration);
}

void AccelSensor::poll() {
    fillBuffer();
}

bool AccelSensor::fillBuffer() {
    // Burst read accelerometer: start at OUT_X_L_A (0x28) with auto-increment
    Wire.beginTransmission(0x19);
    Wire.write(0x28 | 0x80); // 0x80 for auto-increment
    Wire.endTransmission(false);
    if (Wire.requestFrom((uint8_t) 0x19, (uint8_t) 6) != 6) {
        return false;
    }
    // Assemble 12-bit values (low byte first)
    int16_t x = (Wire.read() | (Wire.read() << 8)) >> 4;
    int16_t y = (Wire.read() | (Wire.read() << 8)) >> 4;
    int16_t z = (Wire.read() | (Wire.read() << 8)) >> 4;
    AccelReading newR{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};

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

double AccelSensor::getMagnitude(const AccelReading &r) const {
    return sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
}

bool AccelSensor::equalReadings(const AccelReading &a, const AccelReading &b) const {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

int AccelSensor::bufferSize() const {
    return sizeof(accelBuffer) / sizeof(accelBuffer[0]);
}

const AccelReading &AccelSensor::getCurrentReading() const {
    return accelBuffer[bufferPosition];
}

const AccelReading &AccelSensor::getPreviousReading() const {
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
