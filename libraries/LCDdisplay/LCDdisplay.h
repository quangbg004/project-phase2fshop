#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <time.h>

class LCDdisplay {
public:
    LCDdisplay(const char* ssid, const char* password);

    void begin();
    void update(); // Cũ: cập nhật dựa trên dữ liệu đã set trước
    void update(int aqi, int temp, int hum, int mode, int dayLeft, bool powerState); // Mới: truyền dữ liệu trực tiếp

    void setPower(bool state);
    void setTempHum(int temp, int hum);
    void setMode(int mode);
    void setDayLeft(int days);
    void setAQI(int value);

private:
    TFT_eSPI tft;
    const char* ssid;
    const char* password;

    bool isPowerOn;
    int mode;
    int dayLeft;
    int aqi;
    int temp;
    int hum;

    void drawAQIRing(int centerX, int centerY, int innerRadius, int outerRadius, int aqi);
    uint16_t getAQIColor(int aqi);
    void drawPowerSymbol(int x, int y, int radius, uint16_t color);
    void drawFanModeSymbol(int x, int y, int radius, int mode, uint16_t color);
    void displayMaintenanceDaysLeft(int x, int y, int daysLeft, uint16_t textColor, uint16_t bgColor);
    void displayDate(int centerX, int y, struct tm *timeinfo);
    void displayTime(int centerX, int y, struct tm *timeinfo, uint16_t color, const GFXfont *font);
    void displayTempHumidity(int centerX, int rectY, int temp, int hum);
};

#endif