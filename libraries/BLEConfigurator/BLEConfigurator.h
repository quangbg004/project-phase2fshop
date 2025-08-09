#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Callback interface để báo về app khi nhận đủ SSID/PASS
class IBLECredsHandler {
public:
  virtual ~IBLECredsHandler() = default;
  virtual void onCredentialsReceived(const String& ssid, const String& pass) = 0;
};

class BLEConfigurator {
public:
  BLEConfigurator(const String& deviceName = "ESP32_Config");
  void begin(IBLECredsHandler* handler);
  void startAdvertising();
  void stopAdvertising();

private:
  String _deviceName;
  IBLECredsHandler* _handler = nullptr;

  BLEServer* _server = nullptr;
  BLEService* _service = nullptr;
  BLECharacteristic* _ssidChar = nullptr;
  BLECharacteristic* _passChar = nullptr;

  static constexpr const char* SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
  static constexpr const char* CHAR_SSID   = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
  static constexpr const char* CHAR_PASS   = "beb5483e-36e1-4688-b7f5-ea07361b26a9";

  String _ssid = "";
  String _pass = "";

  class CharCallbacks : public BLECharacteristicCallbacks {
  public:
    CharCallbacks(BLEConfigurator* owner, bool isSSID): _owner(owner), _isSSID(isSSID) {}
    void onWrite(BLECharacteristic* c) override;
  private:
    BLEConfigurator* _owner;
    bool _isSSID;
  };
};
