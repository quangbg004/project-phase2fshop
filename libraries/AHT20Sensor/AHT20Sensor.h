#ifndef AHT20SENSOR_H
#define AHT20SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

class AHT20Sensor {
  public:
    AHT20Sensor();                
    bool begin();                 
    void readData();              
    int getTemperature();         // Trả về số nguyên
    int getHumidity();            // Trả về số nguyên
    
  private:
    Adafruit_AHTX0 aht;           
    int avgTemp;
    int avgHumidity;
};

#endif
