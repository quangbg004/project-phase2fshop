#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

class WiFiManager {
public:
  WiFiManager(const String& ns = "wifi");
  bool loadCredentials(String& ssid, String& pass);
  void saveCredentials(const String& ssid, const String& pass);
  void clearCredentials();

  bool connect(const String& ssid, const String& pass, uint16_t maxAttempts = 20, uint16_t delayMs = 500);
  bool isConnected() const;
  IPAddress localIP() const;

private:
  String _ns;
  Preferences _prefs;
};
