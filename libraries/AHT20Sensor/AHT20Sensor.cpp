#include "AHT20Sensor.h"

AHT20Sensor::AHT20Sensor() {
  avgTemp = 0;
  avgHumidity = 0;
}

bool AHT20Sensor::begin() {
  if (!aht.begin()) {
    return false; 
  }
  return true; 
}

void AHT20Sensor::readData() {
  const int numSamples = 10;
  float totalTemp = 0;
  float totalHumidity = 0;

  for (int i = 0; i < numSamples; i++) {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    totalTemp += temp.temperature;
    totalHumidity += humidity.relative_humidity;
    delay(100);
  }

  // Làm tròn về số nguyên
  avgTemp = round(totalTemp / numSamples);
  avgHumidity = round(totalHumidity / numSamples);
}

int AHT20Sensor::getTemperature() {
  return avgTemp;
}

int AHT20Sensor::getHumidity() {
  return avgHumidity;
}
