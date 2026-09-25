/*
 * BLE input for Mac Plus emulator.
 * GATT server with a writable characteristic for mouse/keyboard events.
 *
 * Protocol (same binary format as previous WebSocket version):
 *   Mouse:    'M' dx:int8 dy:int8 buttons:uint8  (4 bytes)
 *   Key down: 'K' scancode:uint8                  (2 bytes)
 *   Key up:   'U' scancode:uint8                  (2 bytes)
 */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include "esp_mac.h"
#include "esp_gap_ble_api.h"
#include "ble_input.h"

extern "C" {
#include "tme/mouse.h"
#include "tme/via.h"
}

#define SERVICE_UUID        "931e0100-6858-4e55-b637-c3cfdab5ef5f"
#define CHAR_INPUT_UUID     "931e0101-6858-4e55-b637-c3cfdab5ef5f"

static BLECharacteristic *inputChar = nullptr;
static bool deviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *server) override {
        deviceConnected = true;
        printf("BLE: client connected\n");
    }
    void onDisconnect(BLEServer *server) override {
        deviceConnected = false;
        printf("BLE: client disconnected\n");
        // Restart advertising
        server->getAdvertising()->start();
    }
};

class InputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *ch) override {
        std::string val = ch->getValue();
        const uint8_t *data = (const uint8_t *)val.data();
        size_t len = val.length();
        if (len < 1) return;

        switch (data[0]) {
        case 'M':
            if (len >= 4)
                mouseMove((int8_t)data[1], (int8_t)data[2], data[3] & 1);
            break;
        case 'K':
            if (len >= 2) kbdPushKey(data[1], 0);
            break;
        case 'U':
            if (len >= 2) kbdPushKey(data[1], 1);
            break;
        }
    }
};

void bleInputInit() {
    BLEDevice::init("Maclock Plus");

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    esp_bd_addr_t addr;
    addr[0] = (mac[0] & 0x3F) | 0xC0;
    addr[1] = mac[1];
    addr[2] = mac[2];
    addr[3] = mac[3];
    addr[4] = mac[4];
    addr[5] = mac[5] ^ 0x5A;
    esp_ble_gap_set_rand_addr(addr);

    BLEServer *server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    BLEService *service = server->createService(SERVICE_UUID);

    inputChar = service->createCharacteristic(
        CHAR_INPUT_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    inputChar->setCallbacks(new InputCallbacks());

    service->start();

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    BLEAdvertisementData scanData;
    scanData.setName("Maclock Plus");
    adv->setScanResponseData(scanData);
    adv->addServiceUUID(SERVICE_UUID);
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);
    adv->start();

    printf("BLE: advertising as 'Maclock Plus' %02x:%02x:%02x:%02x:%02x:%02x\n",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}
