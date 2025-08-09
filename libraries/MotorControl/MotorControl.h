#include <Arduino.h>
#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H


enum MotorMode {
  MODE_AUTO,
  MODE_WEAK,
  MODE_MEDIUM,
  MODE_STRONG
};

class MotorControl {
  public:
    MotorControl(uint8_t pwmPin);
    void begin();
    void setMode(MotorMode mode);
    void nextMode();
    MotorMode getMode();
    void update();                    
    void setAutoSpeed(uint8_t duty);
    void stopMotor(); 

  private:
    uint8_t _pwmPin;
    uint8_t _pwmChannel;
    MotorMode _currentMode;

    const uint8_t SPEED_STRONG = 255;
    const uint8_t SPEED_MEDIUM = 180;
    const uint8_t SPEED_WEAK   = 100;
    uint8_t _autoDuty = 0;
};

#endif
