#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <TFT_eSPI.h>
#include <time.h>

// Gợi ý: đảm bảo đã bật LOAD_GFXFF trong TFT_eSPI/User_Setup.h
// #define LOAD_GFXFF

class LCDdisplay {
public:
    LCDdisplay();

    void begin();

    // ==== Các hàm update riêng biệt ====
    void updateDate(String dateString);
    void updateAQI(int aqi);
    void updateTempHum(int temp, int hum);
    void updateMode(int mode);
    void updateDayLeft(int dayLeft);
    void updatePowerState(bool powerState);

    // Đổi: cập nhật đồng hồ bằng thời gian truyền vào từ ngoài
    void updateTime(String hourString, String minString);

    void updateDisplay();

private:
    TFT_eSPI tft;
    String dateStringLCD;
    String hourStringLCD;
    String minStringLCD;
    bool powerStateLCD;
    int modeLCD;
    int dayLeftLCD;
    int aqiLCD;
    int tempLCD;
    int humLCD;

    // --- setters nội bộ ---
    void setPower(bool state);
    void setTempHum(int temp, int hum);
    void setMode(int mode);
    void setDayLeft(int days);
    void setAQI(int value);
    void setTime(String hour, String min);
    void setDate(String date);

    // --- helpers vẽ ---
    uint16_t getAQIColor(int aqi);
    void drawAQIRing(int centerX, int centerY, int innerRadius, int outerRadius, int aqi);
    void drawPowerSymbol(int x, int y, int radius, uint16_t color);
    void drawFanModeSymbol(int x, int y, int radius, int mode, uint16_t color);
    void displayMaintenanceDaysLeft(int x, int y, int daysLeft, uint16_t textColor, uint16_t bgColor);
    void displayDate(int centerX, int y, String dateString);
    void displayTime(int centerX, int y, String hourString, String minString, uint16_t color, const GFXfont *font);
    void displayTempHumidity(int centerX, int rectY, int temp, int hum);
};

#endif
