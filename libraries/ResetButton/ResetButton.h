#pragma once
#include <Arduino.h>

class ResetButton {
public:
  ResetButton(uint8_t pin, uint32_t holdMs = 3000)
    : _pin(pin), _holdMs(holdMs) {}

  void begin();
  // Trả về true đúng **một lần** khi phát hiện giữ đủ lâu
  bool pressedLong();

private:
  uint8_t _pin;
  uint32_t _holdMs;
  bool _tracking = false;
  uint32_t _t0 = 0;
};
