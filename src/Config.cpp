#include "Config.h"
#include <EEPROM.h>
#include <algorithm>
#include <memory>
#include <ConfigManager.h>
#include <debug.h>
#include "LedStrip.h"
#include "AccelSensor.h"

extern AccelSensor accelSensor;
extern LedStrip leds;

namespace cfg {
    uint16_t crawlSpeedMs = DEFAULT_CRAWL_SPEED_MS;
    bool reverseStrip = DEFAULT_REVERSE_STRIP;
    uint16_t hermesSensitivity = DEFAULT_HERMES_SENSITIVITY;
    uint8_t baseHue = DEFAULT_BASE_HUE;
    uint8_t emaAlpha = DEFAULT_EMA_ALPHA;
    uint8_t accelDeadband = DEFAULT_ACCEL_DEADBAND;

    void begin() {
        // reserve enough EEPROM for crawlSpeedMs (2), reverseStrip (1), hermesSensitivity (2), baseHue (1), emaAlpha (1), accelDeadband (1)
        EEPROM.begin(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity)
                     + sizeof(baseHue) + sizeof(emaAlpha) + sizeof(accelDeadband));
        // read back; if never written it will be 0xFFFF
        uint16_t storedCrawlSpeed;
        EEPROM.get(0, storedCrawlSpeed);
        if (storedCrawlSpeed != 0xFFFF) {
            crawlSpeedMs = storedCrawlSpeed;
        }
        // read reverseStrip at offset after crawlSpeedMs
        uint8_t storedReverse;
        EEPROM.get(sizeof(crawlSpeedMs), storedReverse);
        if (storedReverse != 0xFF) {
            reverseStrip = storedReverse != 0;
        }
        // read hermesSensitivity at offset after reverseStrip
        uint16_t storedSensitivity;
        EEPROM.get(sizeof(crawlSpeedMs) + sizeof(reverseStrip), storedSensitivity);
        if (storedSensitivity != 0xFFFF) {
            hermesSensitivity = storedSensitivity;
        }
        uint8_t storedHue;
        EEPROM.get(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity), storedHue);
        if (storedHue != 0xFF) {
            baseHue = storedHue;
        }
        uint8_t storedAlpha;
        EEPROM.get(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity) + sizeof(baseHue),
                   storedAlpha);
        if (storedAlpha != 0xFF) emaAlpha = storedAlpha;

        uint8_t storedDb;
        EEPROM.get(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity)
                   + sizeof(baseHue) + sizeof(emaAlpha),
                   storedDb);
        if (storedDb != 0xFF) accelDeadband = storedDb;
    }

    void registerParameters(std::vector<std::unique_ptr<IConfigParameter>> &list) {
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<uint16_t>(
                "00400001-B5A3-F393-E0A9-E50E24DCCA9E",
                "crawlSpeedMs",
                []() { return crawlSpeedMs; },
                [](uint16_t v) {
                    DEBUG_PRINTF("setCrawlSpeedMs %d\n", v);
                    crawlSpeedMs = v;
                    EEPROM.put(0, crawlSpeedMs);
                    EEPROM.commit();
                    ConfigManager::instance().notifyChangeUint("crawlSpeedMs", crawlSpeedMs);
                }
        )));
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<bool>(
                "00400002-B5A3-F393-E0A9-E50E24DCCA9E",
                "reverseStrip",
                []() { return reverseStrip; },
                [](bool v) {
                    DEBUG_PRINTF("setReverseStrip %d\n", v);
                    reverseStrip = v;
                    uint8_t val = v ? 1 : 0;
                    EEPROM.put(sizeof(crawlSpeedMs), val);
                    EEPROM.commit();
                    ConfigManager::instance().notifyChangeBool("reverseStrip", reverseStrip);
                }
        )));
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<uint16_t>(
                "00400003-B5A3-F393-E0A9-E50E24DCCA9E",
                "hermesSensitivity",
                []() { return hermesSensitivity; },
                [](uint16_t v) {
                    DEBUG_PRINTF("sensitivity %d\n", v);
                    hermesSensitivity = v;
                    EEPROM.put(sizeof(crawlSpeedMs) + sizeof(reverseStrip), v);
                    EEPROM.commit();
                    ConfigManager::instance().notifyChangeUint("hermesSensitivity", hermesSensitivity);
                }
        )));
        // --- command: trigger accelerometer recalibration ---
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<uint8_t>(
                "00400004-B5A3-F393-E0A9-E50E24DCCA9E",   // UUID for "recalibrate"
                "recalibrate",
                []() { return static_cast<uint8_t>(0); },  // always reads 0
                [](uint8_t v) {                             // a non‑zero write starts calibration
                    if (v) {
                        DEBUG_PRINTLN("BLE‑triggered recalibration");
                        leds.showCalibrationPattern();
                        accelSensor.calibrate();
                        leds.showingCalibrationPattern = false;
                    }
                }
        )));
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<uint8_t>(
                "00400005-B5A3-F393-E0A9-E50E24DCCA9E",   // UUID for baseHue
                "baseHue",
                []() { return baseHue; },
                [](uint8_t v) {
                    DEBUG_PRINTF("baseHue %u\n", v);
                    baseHue = v;
                    EEPROM.put(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity), v);
                    EEPROM.commit();
                    ConfigManager::instance().notifyChangeUint("baseHue", baseHue);
                }
        )));
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<uint8_t>(
                "00400006-B5A3-F393-E0A9-E50E24DCCA9E",
                "emaAlpha",
                []() { return emaAlpha; },
                [](uint8_t v) {
                    emaAlpha = v;
                    EEPROM.put(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity)
                               + sizeof(baseHue), v);
                    EEPROM.commit();
                    ConfigManager::instance().notifyChangeUint("emaAlpha", emaAlpha);
                }
        )));
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<uint8_t>(
                "00400007-B5A3-F393-E0A9-E50E24DCCA9E",
                "accelDeadband",
                []() { return accelDeadband; },
                [](uint8_t v) {
                    accelDeadband = v;
                    EEPROM.put(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity)
                               + sizeof(baseHue) + sizeof(emaAlpha), v);
                    EEPROM.commit();
                    ConfigManager::instance().notifyChangeUint("accelDeadband", accelDeadband);
                }
        )));
    }


    // ---- accessors ----
    uint16_t getCrawlSpeedMs() { return crawlSpeedMs; }

    bool getReverseStrip() { return reverseStrip; }

    uint16_t getHermesSensitivity() { return hermesSensitivity; }

    uint8_t getBaseHue() { return baseHue; }

    uint8_t getEmaAlpha() { return emaAlpha; }

    uint8_t getAccelDeadband() { return accelDeadband; }
}