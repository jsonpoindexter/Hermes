#include "debug.h"
#include "Config.h"

// Debug functions controlled by run/debug parameters.
unsigned long before = 0;

void loopDebug() {
    if (cfg::WAIT_FOR_KEYBOARD) {
        pauseOnKeystroke();
    }
    if (cfg::PRINT_LOOP_TIME) {
        unsigned long now = millis();
        DEBUG_PRINTLN(now - before);
        before = millis();
    }
}

void pauseOnKeystroke() {
    if (Serial.available()) {
        // Clear the serial buffer.
        Serial.read();

        DEBUG_PRINTLN("Paused. Strike any key to resume...");

        // Turn all LEDs off.
        // leds.colorOff();  // You'll need to handle access to leds

        // Wait for the next keystroke.
        while (!Serial.available()) {}

        // Clear the serial buffer.
        Serial.read();
    }
}

void checkSuperfastHack() {
    // Implement if needed
}