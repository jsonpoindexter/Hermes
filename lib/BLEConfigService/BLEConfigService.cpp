#include "BLEConfigService.h"
#include "../../include/Config.h"
#include <NimBLEDevice.h>

// static constexpr definitions for linkage
constexpr char BLEConfigService::SERVICE_UUID[];
constexpr char BLEConfigService::CRAWL_CHAR_UUID[];

// helper to log connection events
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());
    }
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.printf("Client disconnected - start advertising\n");
        NimBLEDevice::startAdvertising();
    }
} serverCallbacks;

// helper to handle characteristic events and forward writes into our config
class CrawlCallbacks : public NimBLECharacteristicCallbacks {
public:
    explicit CrawlCallbacks(BLEConfigService* svc): service(svc) {}
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        // log current crawl speed in hex
        auto bytes = pCharacteristic->getValue();
        Serial.printf("%s : onRead(), value:", pCharacteristic->getUUID().toString().c_str());
        for (size_t i = 0; i < bytes.length(); ++i) {
            Serial.printf(" %02X", (uint8_t)bytes[i]);
        }
        Serial.println();
    }
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        // parse and apply new crawl speed
        service->onCrawlWritten(pCharacteristic);
    }
private:
    BLEConfigService* service;
};

BLEConfigService::BLEConfigService()
        : crawlChar(nullptr)
{}

void BLEConfigService::begin() {
    NimBLEDevice::init("Hermes-C3");
    auto* server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallbacks);
    auto* svc    = server->createService(SERVICE_UUID);

    crawlChar = svc->createCharacteristic(
            CRAWL_CHAR_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::WRITE_NR
    );
    // init to current value
    crawlChar->setValue((uint16_t)cfg::getCrawlSpeedMs());

    crawlChar->setCallbacks(new CrawlCallbacks(this));

    svc->start();
    server->getAdvertising()->addServiceUUID(SERVICE_UUID);
    server->getAdvertising()->start();
}

void BLEConfigService::poll() {
    // not strictly needed in NimBLE, but placeholder if you add time-based tasks
}

void BLEConfigService::onCrawlWritten(NimBLECharacteristic* chr) {
    auto val = chr->getValue<uint16_t>();
    cfg::setCrawlSpeedMs(val);
    Serial.printf("BLE → new CRAWL_SPEED_MS = %u\n", val);
    // echo back so clients see updated value
    chr->setValue(cfg::getCrawlSpeedMs());
}