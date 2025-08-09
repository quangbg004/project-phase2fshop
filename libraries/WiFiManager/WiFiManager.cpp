#include "WiFiManager.h"

WiFiManager::WiFiManager(const String& ns): _ns(ns) {}

bool WiFiManager::loadCredentials(String& ssid, String& pass) {
  _prefs.begin(_ns.c_str(), true);
  ssid = _prefs.getString("ssid", "");
  pass = _prefs.getString("pass", "");
  _prefs.end();
  return (ssid.length() && pass.length());
}

void WiFiManager::saveCredentials(const String& ssid, const String& pass) {
  _prefs.begin(_ns.c_str(), false);
  _prefs.putString("ssid", ssid);
  _prefs.putString("pass", pass);
  _prefs.end();
  Serial.println("[WiFiManager] Saved credentials to NVS");
}

void WiFiManager::clearCredentials() {
  _prefs.begin(_ns.c_str(), false);
  _prefs.clear();
  _prefs.end();
  Serial.println("[WiFiManager] Cleared credentials in NVS");
}

bool WiFiManager::connect(const String& ssid, const String& pass, uint16_t maxAttempts, uint16_t delayMs) {
  if (ssid.isEmpty() || pass.isEmpty()) return false;
  Serial.print("[WiFiManager] Connecting to SSID: "); Serial.println(ssid);
  WiFi.begin(ssid.c_str(), pass.c_str());
  uint16_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(delayMs);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WiFiManager] Connected. IP: "); Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("[WiFiManager] Failed to connect");
  return false;
}

bool WiFiManager::isConnected() const { return WiFi.status() == WL_CONNECTED; }
IPAddress WiFiManager::localIP() const { return WiFi.localIP(); }
