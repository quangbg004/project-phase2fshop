#include <Arduino.h>
#include "MotorControl.h"

MotorControl::MotorControl(uint8_t pwmPin) {
  _pwmPin = pwmPin;
  _pwmChannel = 0;
  _currentMode = MODE_AUTO;
}

void MotorControl::begin() {
  ledcSetup(_pwmChannel, 10000, 8);
  ledcAttachPin(_pwmPin, _pwmChannel);
  update();
}

void MotorControl::setMode(MotorMode mode) {
  _currentMode = mode;
  update();
}

void MotorControl::nextMode() {
  _currentMode = static_cast<MotorMode>((_currentMode + 1) % 4);
  update();
}

MotorMode MotorControl::getMode() {
  return _currentMode;
}

void MotorControl::setAutoSpeed(uint8_t sensorValue) {
  _autoDuty = map(sensorValue, 0, 4, 50, 255);
  if (_currentMode == MODE_AUTO) {
    ledcWrite(_pwmChannel, _autoDuty);
  }
}

void MotorControl::update() {
  uint8_t duty = 0;
  switch (_currentMode) {
    case MODE_STRONG:
      duty = SPEED_STRONG;
      break;
    case MODE_MEDIUM:
      duty = SPEED_MEDIUM;
      break;
    case MODE_WEAK:
      duty = SPEED_WEAK;
      break;
    case MODE_AUTO:
      duty = _autoDuty;
      break;
  }
  ledcWrite(_pwmChannel, duty);
}

void MotorControl::stopMotor() {
  ledcWrite(_pwmChannel, 0);
}
