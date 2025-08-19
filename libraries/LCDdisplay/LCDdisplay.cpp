#include "LCDdisplay.h"
#include <math.h>


// ===== Constructors =====
LCDdisplay::LCDdisplay()
: tft(TFT_eSPI()),
  dateStringLCD("--/--"), hourStringLCD("--"), minStringLCD("--"),
  powerStateLCD(true), modeLCD(0), dayLeftLCD(100), aqiLCD(0), tempLCD(30), humLCD(50) {}


// ===== Begin =====
void LCDdisplay::begin() {
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.fillScreen(TFT_BLACK);

    updateAQI(aqiLCD);
    updateDate(dateStringLCD);

    updateTime(hourStringLCD, minStringLCD);

    updateMode(modeLCD);
    updateDayLeft(dayLeftLCD);
    updateTempHum(tempLCD, humLCD);
    updatePowerState(powerStateLCD);
}

// ===== Update pieces =====
void LCDdisplay::updateDate(String dateString) {
    if (powerStateLCD){
        setDate(dateString);
        displayDate(120, 55, dateString);
    }
}

void LCDdisplay::updateAQI(int aqi) {
    if (powerStateLCD) {
        setAQI(aqi);
        drawAQIRing(120, 120, 107, 118, aqiLCD);
    }
}

void LCDdisplay::updateTempHum(int temp, int hum) {
    if (powerStateLCD) {
        setTempHum(temp, hum);
        displayTempHumidity(120, 150, tempLCD, humLCD);
    }
}

void LCDdisplay::updateMode(int mode) {
    if (powerStateLCD) {
        setMode(mode);
        drawFanModeSymbol(60, 90, 20, modeLCD, TFT_WHITE);
    }
}

void LCDdisplay::updateDayLeft(int dayLeft) {
    if (powerStateLCD) {
        setDayLeft(dayLeft);
        displayMaintenanceDaysLeft(130, 100, dayLeftLCD, TFT_CYAN, TFT_BLACK);
    }
}

void LCDdisplay::updatePowerState(bool powerState) {
    setPower(powerState);
    if (!powerStateLCD) {
        drawPowerSymbol(120, 210, 10, TFT_LIGHTGREY);
    } else {
        drawPowerSymbol(120, 210, 10, TFT_DARKGREY);
    }
}

// ===== Time =====
void LCDdisplay::updateTime(String hourString, String minString) {
    setTime(hourString, minString);
    if(powerStateLCD) {
        displayTime(120, 186, hourStringLCD, minStringLCD, TFT_WHITE, &Orbitron_Light_24);
    }
    else {
        displayTime(120, 120, hourStringLCD, minStringLCD, TFT_WHITE, &Orbitron_Light_32);
    }
}

// ===== Redraw all =====
void LCDdisplay::updateDisplay() {
    tft.fillScreen(TFT_BLACK);
    if (powerStateLCD) {
        updateAQI(aqiLCD);
        updateDate(dateStringLCD);
        updateTime(hourStringLCD, minStringLCD);
        updateMode(modeLCD);
        updateDayLeft(dayLeftLCD);
        updateTempHum(tempLCD, humLCD);
        updatePowerState(powerStateLCD);
    }
    else {
        updatePowerState(powerStateLCD);
        updateTime(hourStringLCD, minStringLCD);
    }
}

// ===== Setters =====
void LCDdisplay::setPower(bool state) { powerStateLCD = state; }
void LCDdisplay::setTempHum(int t, int h) { tempLCD = t; humLCD = h; }
void LCDdisplay::setMode(int m) { modeLCD = m; }
void LCDdisplay::setDayLeft(int days) { dayLeftLCD = days; }
void LCDdisplay::setAQI(int value) { aqiLCD = value; }
void LCDdisplay::setTime(String hour, String min) { hourStringLCD = hour; minStringLCD = min; };
void LCDdisplay::setDate(String date) { dateStringLCD = date; };

// ===== Helpers =====
uint16_t LCDdisplay::getAQIColor(int aqi) {
    // Hiện đang test 3 mức 0..2
    aqi = constrain(aqi, 0, 2);
    uint8_t r = map(aqi, 0, 2, 0, 255);
    uint8_t g = map(aqi, 0, 2, 255, 0);
    return tft.color565(r, g, 0);
}

// --- Vẽ ---
void LCDdisplay::drawAQIRing(int centerX, int centerY, int innerRadius, int outerRadius, int aqi) {
    uint16_t color = getAQIColor(aqi);
    for (int r = innerRadius; r <= outerRadius; r++) {
        tft.drawCircle(centerX, centerY, r, color);
    }
}

void LCDdisplay::drawPowerSymbol(int x, int y, int radius, uint16_t color) {
    for (int r = radius - 1; r <= radius + 1; r++) {
        for (int angle = 135; angle <= 405; angle++) {
            float rad = angle * 3.14159f / 180.0f;
            int px = x + r * cosf(rad);
            int py = y - r * sinf(rad);
            tft.drawPixel(px, py, color);
        }
    }
    for (int dx = -1; dx <= 1; dx++) {
        tft.drawLine(x + dx, y - radius - 2, x + dx, y, color);
    }
}

void LCDdisplay::drawFanModeSymbol(int x, int y, int radius, int mode, uint16_t color) {
    tft.fillCircle(x, y, radius, TFT_BLACK);
    tft.drawCircle(x, y, radius, color);
    tft.drawCircle(x, y, radius - 1, color);
    tft.drawCircle(x, y, radius + 1, color);

    if (mode == 0) {
        tft.setTextColor(color);
        tft.setFreeFont(&FreeSansBold12pt7b);
        tft.setCursor(x - 8, y + 8);
        tft.print("A");
    } else if (mode == 1) {
        tft.drawArc(x, y, radius - 5, radius - 15, 270, 330, color, TFT_BLACK);
    } else if (mode == 2) {
        tft.drawArc(x, y, radius - 5, radius - 15, 225, 285, color, TFT_BLACK);
        tft.drawArc(x, y, radius - 5, radius - 15, 45, 105, color, TFT_BLACK);
    } else if (mode == 3) {
        tft.drawArc(x, y, radius - 5, radius - 15, 270, 330, color, TFT_BLACK);
        tft.drawArc(x, y, radius - 5, radius - 15, 30, 90, color, TFT_BLACK);
        tft.drawArc(x, y, radius - 5, radius - 15, 150, 210, color, TFT_BLACK);
    }
}

void LCDdisplay::displayMaintenanceDaysLeft(int x, int y, int daysLeft, uint16_t textColor, uint16_t bgColor) {
    tft.fillRect(x, y - 20, 85, 30, bgColor);
    tft.setTextColor(textColor, bgColor);
    tft.setFreeFont(&FreeSans12pt7b);
    tft.setCursor(x, y);
    tft.print(daysLeft);
    tft.print(" day");
}

void LCDdisplay::displayDate(int centerX, int y, String dateString) {
    tft.setFreeFont(&FreeSans12pt7b);
    int dateWidth = tft.textWidth(dateString);
    tft.fillRect(centerX - dateWidth / 2, y - 18, dateWidth + 4, 24, TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(centerX - dateWidth / 2, y);
    tft.print(dateString);
}

void LCDdisplay::displayTime(int centerX, int y,
                             String hourString, String minString,
                             uint16_t color, const GFXfont *font) {
  tft.setFreeFont(font);
  tft.setTextColor(color, TFT_BLACK);
  tft.setTextWrap(false);

  int wHH = tft.textWidth(hourString);
  int wMM = tft.textWidth(minString);
  int wCL = tft.textWidth(":");
  int fh  = tft.fontHeight();

  const int gapL = 7;
  const int gapR = 7;

  int total = wHH + gapL + wCL + gapR + wMM;

  int boxX = centerX - total/2 - 30;
  int boxY = y - fh;
  int boxW = total + 60;
  int boxH = fh + 6;
  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);

  int x = centerX - total/2;

  // HH
  tft.setCursor(x, y);
  tft.print(hourString);
  x += wHH + gapL;

  // Blink “:”
  static uint32_t tick = 0;
  static bool     on   = true;
  if (millis() - tick >= 250) { tick = millis(); on = !on; }

  int colonBoxX = x;
  int colonBoxY = y - fh;
  int colonBoxW = wCL;
  int colonBoxH = fh + 6;
  tft.fillRect(colonBoxX, colonBoxY, colonBoxW, colonBoxH, TFT_BLACK);
  if (on) {
    tft.setCursor(x, y);
    tft.print(":");
  }
  x += wCL + gapR;

  // MM
  tft.setCursor(x, y);
  tft.print(minString);
}




void LCDdisplay::displayTempHumidity(int centerX, int rectY, int temp, int hum) {
    String envStr = String(temp) + " C " + String(hum) + "%";
    tft.setFreeFont(&FreeSansBold18pt7b);
    int envWidth = tft.textWidth(envStr);
    tft.fillRect(20, rectY - 30, 200, 35, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(centerX - envWidth / 2, rectY);
    tft.print(envStr);
    tft.fillCircle(centerX - 32, rectY - 20, 4, TFT_WHITE);
    tft.fillCircle(centerX - 32, rectY - 20, 2, TFT_BLACK);
}
