#include "AHT20Sensor.h"

AHT20Sensor sensor;

void setup() {
  Serial.begin(115200);
  Serial.println("Đang khởi động cảm biến AHT20...");

  if (!sensor.begin()) {
    Serial.println("Không tìm thấy cảm biến AHT20!");
    while (1);
  }

  Serial.println("Khởi động thành công!");
}

void loop() {
  sensor.readData();

  Serial.print("Nhiệt độ TB: ");
  Serial.print(sensor.getTemperature());
  Serial.println(" °C");

  Serial.print("Độ ẩm TB: ");
  Serial.print(sensor.getHumidity());
  Serial.println(" %");

  delay(2000);
}
