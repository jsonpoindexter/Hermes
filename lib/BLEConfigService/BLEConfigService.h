#pragma once
#include <stdint.h>
#include <NimBLEDevice.h>

class BLEConfigService {
public:
    BLEConfigService();
    void begin();     // call in setup()
    void poll();      // call in loop()

    static void onCrawlWritten(NimBLECharacteristic* chr);

private:
    // UUIDs – generate your own!
    static constexpr char SERVICE_UUID[]      = "12345678-1234-4321-ABCD-1234567890AB";
    static constexpr char CRAWL_CHAR_UUID[]   = "87654321-4321-1234-DCBA-BA0987654321";

    NimBLECharacteristic* crawlChar;
};