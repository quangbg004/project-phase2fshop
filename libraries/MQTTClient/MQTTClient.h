#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

class MQTTClient {
public:
  using MessageHandler = std::function<void(const String& topic, const String& payload)>;

  MQTTClient(const char* host, uint16_t port);
  void setAuth(const char* user, const char* password);
  void setMessageHandler(MessageHandler h);

  void begin(); // set server & callback
  bool ensureConnected(const char* clientId, uint32_t retryDelayMs = 5000);
  void loop();
  bool publish(const String& topic, const String& payload, bool retained=false);
  bool subscribe(const String& topic);

  bool isConnected() const;

private:
  WiFiClientSecure _net;
  PubSubClient _mqtt;
  const char* _host;
  uint16_t _port;
  const char* _user = nullptr;
  const char* _pass = nullptr;
  MessageHandler _handler = nullptr;

  static void _staticCallback(char* topic, byte* payload, unsigned int length);
  void _dispatch(char* topic, byte* payload, unsigned int length);
};
