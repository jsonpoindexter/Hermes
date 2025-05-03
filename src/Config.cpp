#include "Config.h"
#include <EEPROM.h>

namespace cfg {
    uint16_t crawlSpeedMs = DEFAULT_CRAWL_SPEED_MS;
    bool reverseStrip = DEFAULT_REVERSE_STRIP;

    void begin() {
        // reserve enough EEPROM for both crawlSpeedMs (2 bytes) and sleepCycleMs (4 bytes)
        EEPROM.begin(sizeof(crawlSpeedMs) + sizeof(reverseStrip));
        // read back; if never written it will be 0xFFFF
        uint16_t stored;
        EEPROM.get(0, stored);
        if (stored != 0xFFFF) {
            crawlSpeedMs = stored;
        }
        // read sleepCycleMs at offset after crawlSpeedMs
        uint32_t storedSleep;
        EEPROM.get(sizeof(crawlSpeedMs), storedSleep);
        if (storedSleep != 0xFFFFFFFF) {
            reverseStrip = storedSleep;
        }
    }

    uint16_t getCrawlSpeedMs() {
        return crawlSpeedMs;
    }

    void setCrawlSpeedMs(uint16_t ms) {
        Serial.printf("setCrawlSpeedMs %d\n", ms);
        crawlSpeedMs = ms;
        EEPROM.put(0, crawlSpeedMs);
        EEPROM.commit();
    }

    bool getReverseStrip() {
        return reverseStrip;
    }

    void setReverseStrip(bool reverse) {
        Serial.printf("setReverseStrip %d\n", reverse);
        reverseStrip = reverse;
        EEPROM.put(sizeof(crawlSpeedMs), reverseStrip);
        EEPROM.commit();
    }
}