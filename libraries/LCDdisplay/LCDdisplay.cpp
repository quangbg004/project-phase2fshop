#include "LCDdisplay.h"

LCDdisplay::LCDdisplay(const char* ssid, const char* password)
: tft(TFT_eSPI()), ssidLCD(ssid), passwordLCD(password),
  isPowerOnLCD(true), modeLCD(0), dayLeftLCD(100), aqiLCD(0), tempLCD(30), humLCD(50) {}

void LCDdisplay::begin() {
    Serial.begin(115200);
    pinMode(22, OUTPUT);
    digitalWrite(22, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    WiFi.begin(ssidLCD, passwordLCD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    configTime(7 * 3600, 0, "pool.ntp.org");

    updateDisplay(); // Vẽ màn hình lần đầu
}

void LCDdisplay::updateAQI(int aqi) {
    setAQI(aqi);
    drawAQIRing(120, 120, 107, 118, aqiLCD);
}

void LCDdisplay::updateTempHum(int temp, int hum) {
    setTempHum(temp, hum);
    displayTempHumidity(120, 150, tempLCD, humLCD);
}

void LCDdisplay::updateMode(int mode) {
    setMode(mode);
    drawFanModeSymbol(60, 90, 20, modeLCD, TFT_WHITE);
}

void LCDdisplay::updateDayLeft(int dayLeft) {
    setDayLeft(dayLeft);
    displayMaintenanceDaysLeft(130, 100, dayLeftLCD, TFT_CYAN, TFT_BLACK);
}

void LCDdisplay::updatePowerState(bool powerState) {
    setPower(powerState);
    if (!powerState) {
        drawPowerSymbol(120, 210, 10, TFT_LIGHTGREY);
    } else {
        drawPowerSymbol(120, 210, 10, TFT_DARKGREY);
    }
}

void LCDdisplay::updateTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Không lấy được thời gian");
        return;
    }
    // Gọi hàm vẽ thời gian bạn đã định nghĩa sẵn
    displayTime(120, 186, &timeinfo, TFT_WHITE, &Orbitron_Light_24);
}

void LCDdisplay::updateDisplay() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Không lấy được thời gian");
        return;
    }

    tft.fillScreen(TFT_BLACK); // Xóa màn hình trước khi vẽ lại toàn bộ

    drawAQIRing(120, 120, 107, 118, aqiLCD);
    displayDate(120, 55, &timeinfo);
    drawFanModeSymbol(60, 90, 20, modeLCD, TFT_WHITE);
    displayMaintenanceDaysLeft(130, 100, dayLeftLCD, TFT_CYAN, TFT_BLACK);
    displayTempHumidity(120, 150, tempLCD, humLCD);
    displayTime(120, 186, &timeinfo, TFT_WHITE, &Orbitron_Light_24);

    if (isPowerOnLCD) {
        drawPowerSymbol(120, 210, 10, TFT_DARKGREY);
    } else {
        drawPowerSymbol(120, 210, 10, TFT_LIGHTGREY);
    }
}

// --- Hàm set nội bộ ---
void LCDdisplay::setPower(bool state) { isPowerOnLCD = state; }
void LCDdisplay::setTempHum(int t, int h) { tempLCD = t; humLCD = h; }
void LCDdisplay::setMode(int m) { modeLCD = m; }
void LCDdisplay::setDayLeft(int days) { dayLeftLCD = days; }
void LCDdisplay::setAQI(int value) { aqiLCD = value; }

uint16_t LCDdisplay::getAQIColor(int aqi) {
    aqi = constrain(aqi, 0, 2);
    uint8_t r = map(aqi, 0, 2, 0, 255);
    uint8_t g = map(aqi, 0, 2, 255, 0);
    return tft.color565(r, g, 0);
}

// --- Các hàm vẽ ---
void LCDdisplay::drawAQIRing(int centerX, int centerY, int innerRadius, int outerRadius, int aqi) {
    uint16_t color = getAQIColor(aqi);
    for (int r = innerRadius; r <= outerRadius; r++) {
        tft.drawCircle(centerX, centerY, r, color);
    }
}

void LCDdisplay::drawPowerSymbol(int x, int y, int radius, uint16_t color) {
    for (int r = radius - 1; r <= radius + 1; r++) {
        for (int angle = 135; angle <= 405; angle++) {
            float rad = angle * 3.14159 / 180.0;
            int px = x + r * cos(rad);
            int py = y - r * sin(rad);
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

void LCDdisplay::displayDate(int centerX, int y, struct tm *timeinfo) {
    char dateStr[16];
    strftime(dateStr, sizeof(dateStr), "%d/%m", timeinfo);
    String dateString = String(dateStr);
    tft.setFreeFont(&FreeSans12pt7b);
    int dateWidth = tft.textWidth(dateString);
    tft.fillRect(centerX - dateWidth / 2, y + 18, dateWidth + 4, 24, TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(centerX - dateWidth / 2, y);
    tft.print(dateString);
}

void LCDdisplay::displayTime(int centerX, int y, struct tm *timeinfo, uint16_t color, const GFXfont *font) {
    char hourStr[3];
    char minStr[3];
    strftime(hourStr, sizeof(hourStr), "%H", timeinfo);
    strftime(minStr, sizeof(minStr), "%M", timeinfo);

    String hourString = String(hourStr);
    String minString  = String(minStr);

    tft.setFreeFont(font);
    int hourWidth = tft.textWidth(hourString);
    int minWidth = tft.textWidth(minString);

    tft.fillRect(centerX - hourWidth + minWidth - 10, y - 18, hourWidth + minWidth + 10, 24, TFT_BLACK);
    tft.setTextColor(color, TFT_BLACK);
    tft.setCursor(centerX - hourWidth - 5, y);
    tft.print(hourString);
    tft.setCursor(centerX + 9, y);
    tft.print(minString);
    delay(200);
    tft.setCursor(centerX, y);
    tft.print(':');
}

void LCDdisplay::displayTempHumidity(int centerX, int rectY, int temp, int hum) {
    String envStr = String(temp) + " C " + String(hum) + "%";
    tft.setFreeFont(&FreeSansBold18pt7b);
    int envWidth = tft.textWidth(envStr);
    tft.fillRect(centerX - envWidth / 2 - 10, rectY - 30, envWidth + 30, 35, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(centerX - envWidth / 2, rectY);
    tft.print(envStr);
    tft.fillCircle(centerX - 32, rectY - 20, 4, TFT_WHITE);
    tft.fillCircle(centerX - 32, rectY - 20, 2, TFT_BLACK);
}
