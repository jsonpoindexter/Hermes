#include "Config.h"
#include <EEPROM.h>

namespace cfg {
    uint16_t crawlSpeedMs = DEFAULT_CRAWL_SPEED_MS;
    uint32_t sleepCycleMs = cfg::SLEEP_CYCLE_MS;

    void begin() {
        // reserve enough EEPROM for both crawlSpeedMs (2 bytes) and sleepCycleMs (4 bytes)
        EEPROM.begin(sizeof(crawlSpeedMs) + sizeof(sleepCycleMs));
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
            sleepCycleMs = storedSleep;
        }
    }

    uint16_t getCrawlSpeedMs() {
        return crawlSpeedMs;
    }

    void setCrawlSpeedMs(uint16_t ms) {
        crawlSpeedMs = ms;
        EEPROM.put(0, crawlSpeedMs);
        EEPROM.commit();
    }

    uint32_t getSleepCycleMs() {
        return sleepCycleMs;
    }

    void setSleepCycleMs(uint32_t ms) {
        sleepCycleMs = ms;
        EEPROM.put(sizeof(crawlSpeedMs), sleepCycleMs);
        EEPROM.commit();
    }
}