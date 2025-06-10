#include "Config.h"
#include <EEPROM.h>
#include <algorithm>
#include <memory>
#include <ConfigManager.h>
#include <debug.h>

namespace cfg {
    uint16_t crawlSpeedMs = DEFAULT_CRAWL_SPEED_MS;
    bool reverseStrip = DEFAULT_REVERSE_STRIP;
    uint16_t hermesSensitivity = DEFAULT_HERMES_SENSITIVITY;

    void begin() {
        // reserve enough EEPROM for both crawlSpeedMs (2 bytes), reverseStrip (1 byte) and hermesSensitivity (2 bytes)
        EEPROM.begin(sizeof(crawlSpeedMs) + sizeof(reverseStrip) + sizeof(hermesSensitivity));
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
    }

    // ---- accessors ----
    uint16_t getCrawlSpeedMs() { return crawlSpeedMs; }

    bool getReverseStrip() { return reverseStrip; }

    uint16_t getHermesSensitivity() { return hermesSensitivity; }
}