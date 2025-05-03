#include "Config.h"
#include <EEPROM.h>
#include <algorithm>
#include <memory>
#include <ConfigManager.h>

namespace cfg {
    uint16_t crawlSpeedMs = DEFAULT_CRAWL_SPEED_MS;
    bool reverseStrip = DEFAULT_REVERSE_STRIP;

    void begin() {
        // reserve enough EEPROM for both crawlSpeedMs (2 bytes) and reverseStrip (1 byte)
        EEPROM.begin(sizeof(crawlSpeedMs) + sizeof(reverseStrip));
        // read back; if never written it will be 0xFFFF
        uint16_t stored;
        EEPROM.get(0, stored);
        if (stored != 0xFFFF) {
            crawlSpeedMs = stored;
        }
        // read reverseStrip at offset after crawlSpeedMs
        uint8_t storedReverse;
        EEPROM.get(sizeof(crawlSpeedMs), storedReverse);
        if (storedReverse != 0xFF) {
            reverseStrip = storedReverse != 0;
        }
    }

    void registerParameters(std::vector<std::unique_ptr<IConfigParameter>>& list) {
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<uint16_t>(
            "6E400001-B5A3-F393-E0A9-E50E24DCCA9E",
            "crawlSpeedMs",
            []() { return crawlSpeedMs; },
            [](uint16_t v) {
                Serial.printf("setCrawlSpeedMs %d\n", v);
                crawlSpeedMs = v;
                EEPROM.put(0, crawlSpeedMs);
                EEPROM.commit();
                ConfigManager::instance().notifyChangeUint("crawlSpeedMs", crawlSpeedMs);
            }
        )));
        list.push_back(std::unique_ptr<IConfigParameter>(new ConfigParameter<bool>(
            "6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
            "reverseStrip",
            []() { return reverseStrip; },
            [](bool v) {
                Serial.printf("setReverseStrip %d\n", v);
                reverseStrip = v;
                uint8_t val = v ? 1 : 0;
                EEPROM.put(sizeof(crawlSpeedMs), val);
                EEPROM.commit();
                ConfigManager::instance().notifyChangeBool("reverseStrip", reverseStrip);
            }
        )));
    }

    // ---- accessors ----
    uint16_t getCrawlSpeedMs() { return crawlSpeedMs; }
    bool     getReverseStrip() { return reverseStrip; }
}