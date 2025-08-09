#include "MQTTClient.h"

static MQTTClient* _singletonForCb = nullptr;

MQTTClient::MQTTClient(const char* host, uint16_t port)
  : _mqtt(_net), _host(host), _port(port) {
  _singletonForCb = this;
}

void MQTTClient::setAuth(const char* user, const char* password) {
  _user = user; _pass = password;
}

void MQTTClient::setMessageHandler(MessageHandler h) { _handler = h; }

void MQTTClient::begin() {
  _net.setInsecure();             // demo: bỏ CA; thực tế nên set CA
  _mqtt.setServer(_host, _port);
  _mqtt.setCallback(MQTTClient::_staticCallback);
}

bool MQTTClient::ensureConnected(const char* clientId, uint32_t retryDelayMs) {
  if (_mqtt.connected()) return true;
  Serial.print("[MQTT] Connecting...");
  if (_mqtt.connect(clientId, _user, _pass)) {
    Serial.println("connected.");
    return true;
  }
  Serial.print("failed, rc="); Serial.println(_mqtt.state());
  delay(retryDelayMs);
  return false;
}

void MQTTClient::loop() { _mqtt.loop(); }

bool MQTTClient::publish(const String& topic, const String& payload, bool retained) {
  if (!_mqtt.connected()) return false;
  return _mqtt.publish(topic.c_str(), payload.c_str(), retained);
}

bool MQTTClient::subscribe(const String& topic) {
  if (!_mqtt.connected()) return false;
  return _mqtt.subscribe(topic.c_str());
}

bool MQTTClient::isConnected() const { return _mqtt.connected(); }

void MQTTClient::_staticCallback(char* topic, byte* payload, unsigned int length) {
  if (_singletonForCb) _singletonForCb->_dispatch(topic, payload, length);
}

void MQTTClient::_dispatch(char* topic, byte* payload, unsigned int length) {
  if (!_handler) return;
  String t = String(topic);
  String p; p.reserve(length);
  for (unsigned int i = 0; i < length; ++i) p += (char)payload[i];
  _handler(t, p);
}
