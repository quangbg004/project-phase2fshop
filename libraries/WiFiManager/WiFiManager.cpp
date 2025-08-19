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

  if (ssid.isEmpty()) {
    Serial.println("[WiFiManager] SSID is empty — skip");
    return false;
  }

  wl_status_t st = WiFi.status();
  if (st == WL_CONNECTED) return true;

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  static bool inProgress = false;
  static uint32_t beginAtMs = 0;
  static uint32_t lastLogMs = 0;
  static uint8_t windowAttempts = 0;
  static String currentSsid;
  static String currentPass;

  auto printStatus = [](wl_status_t s) {
    switch (s) {
      case WL_IDLE_STATUS:    Serial.print("IDLE"); break;
      case WL_NO_SSID_AVAIL:  Serial.print("NO_SSID"); break;
      case WL_SCAN_COMPLETED: Serial.print("SCAN_DONE"); break;
      case WL_CONNECTED:      Serial.print("CONNECTED"); break;
      case WL_CONNECT_FAILED: Serial.print("AUTH_FAIL"); break;
      case WL_CONNECTION_LOST:Serial.print("LOST"); break;
      case WL_DISCONNECTED:   Serial.print("DISCONNECTED"); break;
      default:                Serial.print((int)s); break;
    }
  };

  const uint32_t now = millis();
  const uint32_t ATTEMPT_TIMEOUT_MS = 8000;
  const uint32_t BACKOFF_MS = 2000;

  if (!inProgress) {
    currentSsid = ssid;
    currentPass = pass;
    Serial.print("[WiFiManager] Connecting to SSID: "); Serial.println(currentSsid);
    WiFi.begin(currentSsid.c_str(), currentPass.c_str());
    inProgress = true;
    beginAtMs = now;
    lastLogMs = 0;
    windowAttempts++;
  }

  if (now - lastLogMs >= 1000) {
    Serial.print("[WiFiManager] status=");
    printStatus(WiFi.status());
    Serial.print("  RSSI="); Serial.println(WiFi.RSSI());
    lastLogMs = now;
  }

  st = WiFi.status();
  if (st == WL_CONNECTED) {
    Serial.print("[WiFiManager] Connected. IP: "); Serial.println(WiFi.localIP());
    inProgress = false;
    beginAtMs = 0;
    windowAttempts = 0;
    return true;
  }

  if (now - beginAtMs >= ATTEMPT_TIMEOUT_MS) {
    Serial.println("[WiFiManager] Attempt timeout");
    inProgress = false; // allow a new attempt window next call

    static uint32_t nextAllowedMs = 0;
    if (now < nextAllowedMs) return false;
    nextAllowedMs = now + BACKOFF_MS;

    if (maxAttempts > 0 && windowAttempts >= maxAttempts) {
      Serial.println("[WiFiManager] Failed to connect (maxAttempts reached)");
      windowAttempts = 0;
      return false;
    }
  }

  return false;
}


bool WiFiManager::isConnected() const { return WiFi.status() == WL_CONNECTED; }
IPAddress WiFiManager::localIP() const { return WiFi.localIP(); }
