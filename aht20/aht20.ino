#include "MotorControl.h"
#include "mq135_sensor.h"
#include "AHT20Sensor.h"
// #include "LCDdisplay.h"

#define BUTTON_PIN 25   // Nút đổi chế độ
#define BUTTON_ON  26   // Nút bật/tắt ưu tiên cao
bool pbto = true;
// LCDdisplay lcd("Oasis 157", "123456789");
AHT20Sensor sensor;
MotorControl motor(14);
MQ135_Data sensor_mq135;

volatile bool flagChangeMode = false;
volatile bool flagTogglePower = false;

volatile unsigned long lastPressMode = 0;
volatile unsigned long lastPressPower = 0;

bool IsPowerOn = true;

// Chuyển enum sang chuỗi
const char* modeToStr(MotorMode mode) {
  switch (mode) {
    case MODE_WEAK:   return "WEAK";
    case MODE_MEDIUM: return "MEDIUM";
    case MODE_STRONG: return "STRONG";
    case MODE_AUTO:   return "AUTO";
    default:          return "UNKNOWN";
  }
}

// ISR cho nút đổi chế độ
void IRAM_ATTR isrChangeMode() {
  unsigned long now = millis();
  if (now - lastPressMode > 200) { // debounce 200ms
    flagChangeMode = true;
    lastPressMode = now;
  }
}

// ISR cho nút bật/tắt
void IRAM_ATTR isrTogglePower() {
  unsigned long now = millis();
  if (now - lastPressPower > 200) {
    flagTogglePower = true;
    lastPressPower = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!sensor.begin()) {
    Serial.println("Không tìm thấy cảm biến AHT20!");
    while (1);
  }

  mq135_init();
  motor.begin();
  motor.setMode(MODE_AUTO);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isrChangeMode, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_ON), isrTogglePower, FALLING);

  Serial.print("Khởi động ở chế độ: ");
  Serial.println(modeToStr(motor.getMode()));

  // lcd.begin();
}

void loop() {
  // ===== Xử lý nút bật/tắt =====
  if (flagTogglePower) {
    IsPowerOn = !IsPowerOn;
    pbto=!pbto;
    if (!IsPowerOn) {
      motor.stopMotor();
      Serial.println("Động cơ đã TẮT");
    } else {
      Serial.println("Động cơ đã BẬT lại");
      motor.setMode(MODE_AUTO);
    }
    flagTogglePower = false;
  }

  // ===== Xử lý nút đổi chế độ =====
  if (flagChangeMode && IsPowerOn) {
    motor.nextMode();
    Serial.print("Chuyển sang chế độ: ");
    Serial.println(modeToStr(motor.getMode()));
    flagChangeMode = false;
  }

  // ===== Đọc cảm biến và điều khiển tốc độ =====
  if (IsPowerOn) {
    sensor_mq135 = mq135_read();
    if (motor.getMode() == MODE_AUTO) {
      motor.setAutoSpeed(sensor_mq135.canhBao);
    }
  }
  

  // ===== Đọc nhiệt độ & độ ẩm =====
  sensor.readData();
  int temp = sensor.getTemperature();
  int hum = sensor.getHumidity();
  // int aqi = sensor_mq135.canhBao;

  // lcd.update(aqi, temp, hum, motor.getMode(), 99, pbto);

  Serial.print("Nhiệt độ TB: ");
  Serial.print(temp);
  Serial.println(" °C");

  Serial.print("Độ ẩm TB: ");
  Serial.print(hum);
  Serial.println(" %");

  delay(200);
}
