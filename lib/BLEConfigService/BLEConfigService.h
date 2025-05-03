#pragma once
#include <stdint.h>
#include <NimBLEDevice.h>
#include <functional>
#include <vector>

struct ConfigEntry {
  const char* uuid;
  const char* name;
  size_t size;
  std::function<uint32_t()> getter;
  std::function<void(uint32_t)> setter;
};

class BLEConfigService {
public:
    BLEConfigService();
    void begin();     // call in setup()
    void poll();      // call in loop()
    static constexpr char SERVICE_UUID[]    = "12345678-1234-4321-ABCD-1234567890AB";
    static constexpr char CRAWL_CHAR_UUID[] = "87654321-4321-1234-DCBA-BA0987654321";
    static constexpr char REVERSE_STRIP_CHAR_UUID[] = "ABCDEFAB-CDEF-ABCD-EFAB-CDEFABCDEFAB";
private:
    std::vector<ConfigEntry> entries;
    std::vector<NimBLECharacteristic*> chars;
};