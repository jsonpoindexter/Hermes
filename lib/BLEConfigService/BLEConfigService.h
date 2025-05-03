#pragma once
#include <stdint.h>
#include <NimBLEDevice.h>
#include <functional>
#include <vector>
#include <memory>

struct IConfigParameter;

class BLEConfigService {
public:
    BLEConfigService();
    void begin();     // call in setup()
    void poll();      // call in loop()
    static constexpr char SERVICE_UUID[]    = "12345678-1234-4321-ABCD-1234567890AB";
private:
    std::vector<std::unique_ptr<IConfigParameter>> entries;
    std::vector<NimBLECharacteristic*> chars;
};