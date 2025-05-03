#include <NimBLEDevice.h>
#include "BLEConfigService.h"
#include "Config.h"
#include <cstring>
#include <memory>
#include <vector>

// static constexpr definitions for linkage
constexpr char BLEConfigService::SERVICE_UUID[];

// constructor
BLEConfigService::BLEConfigService() = default;

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
    explicit GenericParamCallbacks(IConfigParameter* e): entry(e) {}

    // single-parameter onRead
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        uint8_t buf[8] = {0};
        entry->read(buf);
        pCharacteristic->setValue(buf, entry->size());
        Serial.printf("%s onRead\n", entry->name());
    }

    // single-parameter onWrite
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        auto raw = pCharacteristic->getValue();
        entry->write(raw.data(), raw.size());
        Serial.printf("%s onWrite\n", entry->name());
        // echo back the new value
        uint8_t buf[8] = {0};
        entry->read(buf);
        pCharacteristic->setValue(buf, entry->size());
    }

private:
    IConfigParameter* entry;
};

void BLEConfigService::begin() {
    NimBLEDevice::init("Hermes-C3");
    auto* server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallbacks);
    auto* svc    = server->createService(SERVICE_UUID);

    // populate entries
    entries.clear();
    cfg::registerParameters(entries);

    // create one characteristic per entry
    for (auto& e : entries) {
        auto* chr = svc->createCharacteristic(
                e->uuid(),
                NIMBLE_PROPERTY::READ |
                NIMBLE_PROPERTY::WRITE |
                NIMBLE_PROPERTY::WRITE_NR
        );
        uint8_t buf[8] = {0}; // enough size for all parameters
        e->read(buf);
        chr->setValue(buf, e->size());
        chr->setCallbacks(new GenericParamCallbacks(e.get()));
        chars.push_back(chr);
    }

    svc->start();
    server->getAdvertising()->addServiceUUID(SERVICE_UUID);
    server->getAdvertising()->start();

    Serial.println("BLEConfigService started");
}