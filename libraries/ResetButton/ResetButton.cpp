#include "ResetButton.h"

void ResetButton::begin() {
  pinMode(_pin, INPUT_PULLUP);
}

bool ResetButton::pressedLong() {
  int level = digitalRead(_pin); // LOW khi nhấn
  if (level == LOW && !_tracking) {
    _tracking = true; _t0 = millis();
  } else if (level == LOW && _tracking) {
    if (millis() - _t0 >= _holdMs) {
      while (digitalRead(_pin) == LOW) delay(10);
      _tracking = false;
      return true;
    }
  } else {
    _tracking = false;
  }
  return false;
}
