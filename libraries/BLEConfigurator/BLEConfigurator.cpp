#include "BLEConfigurator.h"

BLEConfigurator::BLEConfigurator(const String& deviceName): _deviceName(deviceName) {}

void BLEConfigurator::begin(IBLECredsHandler* handler) {
    if (_started) return; // tránh start lại
    _started = true;

    _handler = handler;
    static bool inited = false;
    if (!inited) {
        BLEDevice::init(_deviceName.c_str());
        inited = true;
    }

    _server  = BLEDevice::createServer();
    _service = _server->createService(SERVICE_UUID);

    _ssidChar = _service->createCharacteristic(CHAR_SSID, BLECharacteristic::PROPERTY_WRITE);
    _passChar = _service->createCharacteristic(CHAR_PASS, BLECharacteristic::PROPERTY_WRITE);

    _ssidChar->setCallbacks(new CharCallbacks(this, true));
    _passChar->setCallbacks(new CharCallbacks(this, false));

    _service->start();

    auto adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID); // để app scan đúng service
    adv->setScanResponse(true);

    startAdvertising();
    Serial.println("[BLEConfigurator] BLE started, waiting for Wi-Fi credentials...");
}



void BLEConfigurator::startAdvertising() {
  BLEDevice::getAdvertising()->start();
}

void BLEConfigurator::stopAdvertising() {
    BLEDevice::getAdvertising()->stop();
    _started = false;
}


void BLEConfigurator::CharCallbacks::onWrite(BLECharacteristic* c) {
  std::string v = c->getValue();
  if (_isSSID) {
    _owner->_ssid = String(v.c_str());
    Serial.println("[BLEConfigurator] SSID received: " + _owner->_ssid);
  } else {
    _owner->_pass = String(v.c_str());
    Serial.println("[BLEConfigurator] Password received (hidden)");
  }
  if (_owner->_ssid.length() && _owner->_pass.length()) {
    // đủ credentials
    _owner->stopAdvertising();
    Serial.println("[BLEConfigurator] Credentials complete, advertising stopped");
    if (_owner->_handler) _owner->_handler->onCredentialsReceived(_owner->_ssid, _owner->_pass);
  }
}
