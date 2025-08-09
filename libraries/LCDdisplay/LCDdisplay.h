#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <time.h>

class LCDdisplay {
public:
    LCDdisplay(const char* ssid, const char* password);

    void begin();

    // Các hàm update riêng biệt
    void updateAQI(int aqi);
    void updateTempHum(int temp, int hum);
    void updateMode(int mode);
    void updateDayLeft(int dayLeft);
    void updatePowerState(bool powerState);
    void updateTime();

    // Hàm cập nhật lại màn hình tổng quan
    void updateDisplay();

private:
    TFT_eSPI tft;
    const char* ssidLCD;
    const char* passwordLCD;

    bool isPowerOnLCD;
    int modeLCD;
    int dayLeftLCD;
    int aqiLCD;
    int tempLCD;
    int humLCD;

    void setPower(bool state);
    void setTempHum(int temp, int hum);
    void setMode(int mode);
    void setDayLeft(int days);
    void setAQI(int value);

    uint16_t getAQIColor(int aqi);

    void drawAQIRing(int centerX, int centerY, int innerRadius, int outerRadius, int aqi);
    void drawPowerSymbol(int x, int y, int radius, uint16_t color);
    void drawFanModeSymbol(int x, int y, int radius, int mode, uint16_t color);
    void displayMaintenanceDaysLeft(int x, int y, int daysLeft, uint16_t textColor, uint16_t bgColor);
    void displayDate(int centerX, int y, struct tm *timeinfo);
    void displayTime(int centerX, int y, struct tm *timeinfo, uint16_t color, const GFXfont *font);
    void displayTempHumidity(int centerX, int rectY, int temp, int hum);
};

#endif
