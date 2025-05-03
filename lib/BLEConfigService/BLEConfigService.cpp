#include <NimBLEDevice.h>
#include "BLEConfigService.h"
#include "../../include/Config.h"
#include <cstring>

// static constexpr definitions for linkage
constexpr char BLEConfigService::SERVICE_UUID[];
constexpr char BLEConfigService::CRAWL_CHAR_UUID[];
constexpr char BLEConfigService::REVERSE_STRIP_CHAR_UUID[];

// constructor
BLEConfigService::BLEConfigService() {}

// build a table of all BLE-exposed config params
static const ConfigEntry PARAMS[] = {
  { BLEConfigService::CRAWL_CHAR_UUID,
    "[Crawl Speed (ms)]",
    sizeof(cfg::DEFAULT_CRAWL_SPEED_MS),
    [](){ return cfg::getCrawlSpeedMs(); },
    [](uint16_t v){ cfg::setCrawlSpeedMs(v); }
  },
  { BLEConfigService::REVERSE_STRIP_CHAR_UUID,
    "[Reverse Strip]",
    sizeof(cfg::DEFAULT_REVERSE_STRIP),
    [](){ return cfg::getReverseStrip(); },
    [](bool v){ cfg::setReverseStrip(v); }
  },
};

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());
    }
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.printf("Client disconnected - start advertising\n");
        NimBLEDevice::startAdvertising();
    }
} serverCallbacks;

// generic callbacks for any config param
class GenericParamCallbacks : public NimBLECharacteristicCallbacks {
public:
    explicit GenericParamCallbacks(const ConfigEntry* e): entry(e) {}

    // single-parameter onRead
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        uint32_t val = entry->getter();

        // send back little-endian bytes of the correct size
        pCharacteristic->setValue((uint8_t*)&val, entry->size);
        // Include name
        Serial.printf("%s onRead %08X\n",
           entry->name, val);
    }

    // single-parameter onWrite
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        auto raw = pCharacteristic->getValue();
        uint32_t val = 0;
        // copy up to entry->size bytes into val (little-endian)
        size_t len = raw.size() < entry->size ? raw.size() : entry->size;
        memcpy(&val, raw.data(), len);
        entry->setter(val);
        Serial.printf("%s onWrite %08X\n",
            entry->name, val);
        // echo back the new value
        pCharacteristic->setValue((uint8_t*)&val, entry->size);
    }

private:
    const ConfigEntry* entry;
};

void BLEConfigService::begin() {
    NimBLEDevice::init("Hermes-C3");
    auto* server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallbacks);
    auto* svc    = server->createService(SERVICE_UUID);

    // populate entries
    entries.assign(std::begin(PARAMS), std::end(PARAMS));
    // create one characteristic per entry
    for (auto& e : entries) {
      auto* chr = svc->createCharacteristic(
        e.uuid,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_NR
      );
      uint32_t v = e.getter();
      chr->setValue(reinterpret_cast<const uint8_t*>(&v), sizeof(v));
      chr->setCallbacks(new GenericParamCallbacks(&e));
      chars.push_back(chr);
    }

    svc->start();
    server->getAdvertising()->addServiceUUID(SERVICE_UUID);
    server->getAdvertising()->start();

    Serial.println("BLEConfigService started");
}

void BLEConfigService::poll() {
    // not strictly needed in NimBLE, but placeholder if you add time-based tasks
}