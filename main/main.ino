#include "MotorControl.h"
#include "mq135_sensor.h"
#include "AHT20Sensor.h"
#include <Arduino.h>
#include "WiFiManager.h"
#include "BLEConfigurator.h"
#include "MQTTClient.h"
#include "ResetButton.h"
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include "LCDdisplay.h"
#include <Preferences.h>

LCDdisplay lcd;

#define BUTTON_PIN 25   // Nút đổi chế độ
#define BUTTON_ON  26   // Nút bật/tắt ưu tiên cao
static const int RESET_BTN_PIN = 27;
unsigned long lastSendTime = 0;
// MQTT settings 
static const char* MQTT_HOST = "8485296d32114f68a6b11965cfcf6cbc.s1.eu.hivemq.cloud";
static const uint16_t MQTT_PORT = 8883;
static const char* MQTT_USER = "esp32client";
static const char* MQTT_PASS = "Nampro09048";
// Topics                                                                                                                                                                                                                                          88888888888888888
static const char* TOPIC_SUB   = "airpurifier/control";
static const char* TOPIC_PUB   = "esp32/test";
static const char* TOPIC_PUB2  = "esp32/aht20";
static const char* MQTT_CLIENT_ID = "ESP32Client";

// ==== NTP ====
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const char* TIMEZONE_VIETNAM = "ICT-7";
struct tm timeinfo;


// --- DayLeft ---
static const uint16_t FILTER_TARGET_DAYS  = 120;  // số ngày
static const uint16_t FILTER_TARGET_HOURS = 800;  // số giờ

Preferences gPref;
uint64_t gInstallTs = 0;   // epoch lúc thay màng
uint64_t gAccMs     = 0;   // ms quạt chạy
uint32_t gLastMs    = 0;   // mốc trước đó
uint32_t gLastSave  = 0;   // mốc lưu NVS gần nhất
const  uint32_t SAVE_INTERVAL_MS = 60000; // 60s/lần

int dayLeft = FILTER_TARGET_DAYS;
int lastDayLeft  = -1;


// In ngày/giờ dạng String
bool getDateHourMinute(String& dateStr, String& hourStr, String& minStr) {
  struct tm now;
  if (!getLocalTime(&now)) {
    dateStr = F("--/--");
    hourStr = F("--");
    minStr  = F("--");
    return false;
  }

  char bufDate[11], bufH[3], bufM[3];
  strftime(bufDate, sizeof(bufDate), "%d/%m",    &now);
  strftime(bufH,    sizeof(bufH),    "%H",       &now);
  strftime(bufM,    sizeof(bufM),    "%M",       &now);

  dateStr = bufDate;
  hourStr = bufH;
  minStr  = bufM;
  return true;
}


AHT20Sensor sensor;
MotorControl motor(14);
MQ135_Data sensor_mq135;

volatile bool flagChangeMode = false;
volatile bool flagTogglePower = false;

volatile unsigned long lastPressMode = 0;
volatile unsigned long lastPressPower = 0;

bool IsPowerOn = true;

// ====== Globals ======
WiFiManager wifiMgr;
MQTTClient  mqtt(MQTT_HOST, MQTT_PORT);
ResetButton resetBtn(RESET_BTN_PIN, 3000);

String g_ssid, g_pass;
bool g_haveCreds = false;
bool g_bleStarted = false;

// BLE → khi nhận đủ SSID/PASS thì callback
class CredsHandler : public IBLECredsHandler {
public:
  void onCredentialsReceived(const String& ssid, const String& pass) override {
    g_ssid = ssid;
    g_pass = pass;
    wifiMgr.saveCredentials(ssid, pass);
    g_haveCreds = true;
  }
};

BLEConfigurator ble;
CredsHandler credsHandler;

// ====== Helper ======
void startBLEOnce() {
  if (!g_bleStarted) {
    ble.begin(&credsHandler);
    g_bleStarted = true;
  }
}

void stopBLEIfRunning() {
  if (g_bleStarted) {
ble.stopAdvertising();
    g_bleStarted = false;
  }
}

// ====== Publish ======
void sendMessage(const String& s) {
  if (mqtt.isConnected()) {
    mqtt.publish(TOPIC_PUB, s);
    Serial.println(String("[APP] published: ") + s);
  }
}

void sendMessage2(const String& s) {
  if (mqtt.isConnected()) {
    mqtt.publish(TOPIC_PUB2, s);
    Serial.println(String("[APP] published to DHT topic: ") + s);
  }
}

// ====== MQTT Callback ======
void onMqttMessage(const String& topic, const String& payload) {
  Serial.printf("[MQTT] %s -> %s\n", topic.c_str(), payload.c_str());
  if (payload == "on") {
    IsPowerOn = true;
    lcd.updatePowerState(true);
    motor.setMode(MODE_AUTO);
    lcd.updateMode(0);
    lcd.updateDisplay();
    sendMessage("{\"out1\": 1}");
    String s = String("{\"out2\": ") + String(motor.getMode()) + "}";
    sendMessage(s);
  } else if (payload == "off") {
    IsPowerOn=false;
    motor.stopMotor();
    lcd.updatePowerState(false);
    lcd.updateDisplay();
    sendMessage("{\"out1\": 0}");
  } else if (payload == "low" && (IsPowerOn==true)) {
    motor.setMode(MODE_WEAK);
    lcd.updateMode(1);
    String s = String("{\"out2\": ") + String(motor.getMode()) + "}";
    sendMessage(s);
  } else if (payload == "medium" && (IsPowerOn==true)) {
    motor.setMode(MODE_MEDIUM);
    lcd.updateMode(2);
    String s = String("{\"out2\": ") + String(motor.getMode()) + "}";
    sendMessage(s);
  } else if (payload == "high" && (IsPowerOn==true)) {
    motor.setMode(MODE_STRONG);
    lcd.updateMode(3);
    String s = String("{\"out2\": ") + String(motor.getMode()) + "}";
    sendMessage(s);
  } else if (payload == "auto" && (IsPowerOn==true)) {
    motor.setMode(MODE_AUTO);
    lcd.updateMode(0);
    String s = String("{\"out2\": ") + String(motor.getMode()) + "}";
    sendMessage(s);
  } else {
    return;
  }
}

// ====== Config Reset ======
void enterReconfigMode() {
  Serial.println("[APP] Enter reconfig mode: clear NVS, stop MQTT/WiFi, start BLE");
  wifiMgr.clearCredentials();
  g_ssid = ""; g_pass = ""; g_haveCreds = false;
  WiFi.disconnect(true, true);
  startBLEOnce();
}

// ====== WiFi + MQTT Management ======
void ensureWifiMqtt() {
  // 1) Wi-Fi
  if (!wifiMgr.isConnected()) {
    if (!g_haveCreds) {
      if (wifiMgr.loadCredentials(g_ssid, g_pass)) {
        g_haveCreds = true;
        Serial.println("[APP] Loaded credentials from NVS");
      } else {
        startBLEOnce();
        return; // đợi user nhập creds
      }
    }

    // Thử kết nối Wi-Fi theo kiểu "non-blocking" thô: 1 attempt / vòng
    if (!wifiMgr.connect(g_ssid, g_pass, /*maxAttempts=*/1, /*delayMs=*/0)) {
      return; // thử lại vòng sau
    }
  }

  // 2) Đã có Wi-Fi => tắt quảng bá BLE nếu đang bật
  stopBLEIfRunning();

  // 3) MQTT – hạn chế block bằng backoff thủ công
  static uint32_t nextTryMs = 0;
  if (!mqtt.isConnected()) {
    if (millis() >= nextTryMs) {
      if (mqtt.ensureConnected(MQTT_CLIENT_ID, /*retryDelayMs=*/0)) {
        mqtt.subscribe(TOPIC_SUB);
        sendMessage("ESP32 đã kết nối thành công MQTT");
      }
      nextTryMs = millis() + 5000; 
    }
    return;
  }
}



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

// ISR nút đổi chế độ
void IRAM_ATTR isrChangeMode() {
  unsigned long now = millis();
  if (now - lastPressMode > 200) { // debounce 200ms
    flagChangeMode = true;
    lastPressMode = now;
  }
}

// ISR nút bật/tắt
void IRAM_ATTR isrTogglePower() {
  unsigned long now = millis();
  if (now - lastPressPower > 200) {
    flagTogglePower = true;
    lastPressPower = now;
  }
}

static inline bool timeValid() {
  return time(nullptr) > 1000000000;
}

void loadFilterLife() {
  gPref.begin("filter", false);
  gInstallTs = gPref.getULong64("instTs", 0ULL);
  gAccMs     = gPref.getULong64("accMs",  0ULL);
  gPref.end();
}

void saveAccMs() {
  gPref.begin("filter", false);
  gPref.putULong64("accMs", gAccMs);
  gPref.end();
}

void markFilterReplaced() {
  time_t now = time(nullptr);
  uint64_t ts = (timeValid() ? (uint64_t)now : 0ULL);

  gPref.begin("filter", false);
  gPref.putULong64("instTs", ts);
  gPref.putULong64("accMs",  0ULL);
  gPref.end();

  gInstallTs = ts;
  gAccMs     = 0;
}

// Tính số ngày còn lại
int computeDayLeft() {
  if (timeValid() && gInstallTs > 0) {
    time_t now = time(nullptr);
    int usedDays = (int)((now - (time_t)gInstallTs)/86400);
    int left = (int)FILTER_TARGET_DAYS - max(0, usedDays);
    return left > 0 ? left : 0;
  }

  double usedHours = (double)gAccMs / 3600000.0;
  double leftHours = max(0.0, (double)FILTER_TARGET_HOURS - usedHours);
  double k = (double)FILTER_TARGET_DAYS / (double)FILTER_TARGET_HOURS; // hệ số đổi
  int leftDays = (int)lround(leftHours * k);
  return leftDays > 0 ? leftDays : 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Cho phép lấy NTP server từ DHCP
  sntp_servermode_dhcp(1);

  // Cấu hình múi giờ bằng POSIX TZ 
  configTzTime(TIMEZONE_VIETNAM, ntpServer1, ntpServer2);

  WiFi.mode(WIFI_STA);
  // nút reset cấu hình
  resetBtn.begin();

  // MQTT
  mqtt.setAuth(MQTT_USER, MQTT_PASS);
  mqtt.setMessageHandler(onMqttMessage);
  mqtt.begin();

  if (wifiMgr.loadCredentials(g_ssid, g_pass)) {
    g_haveCreds = true;
    Serial.println("[APP] Credentials found in NVS");
  } else {
    Serial.println("[APP] No credentials. Starting BLE config");
    startBLEOnce();
  }
  if (!sensor.begin()) {
    Serial.println("Không tìm thấy cảm biến AHT20!");
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
  lcd.begin();

  // --- Filter life init ---
loadFilterLife();
gLastMs   = millis();
gLastSave = gLastMs;

if (timeValid() && gInstallTs == 0) {
  markFilterReplaced();
}

dayLeft = computeDayLeft();
lcd.updateDayLeft(dayLeft);
}

void loop() {

  // 1) Cho phép reset cấu hình bất kỳ lúc nào
  if (resetBtn.pressedLong()) {
    enterReconfigMode();
  }

  // 2) Đảm bảo WiFi/MQTT luôn được duy trì
  ensureWifiMqtt();

  // 3) MQTT loop
  mqtt.loop();
  
  String lastMin = "-1";
  String lastDate = "--/--";
  String dateStr;
  String hourStr;
  String minStr;

  if (getDateHourMinute(dateStr, hourStr, minStr)) {
    Serial.printf("VN Date: %s | VN Time: %s:%s\n", dateStr, hourStr, minStr);
    
    if (minStr != lastMin) {
        lcd.updateTime(hourStr, minStr);
        lastMin = minStr;
    }
    if (dateStr != lastDate) {
      lcd.updateDate(dateStr);
      lastDate = dateStr;
    } 
    
  } else {
    Serial.println("No time available (yet)");
  }

  uint32_t nowMs = millis();
  uint32_t dt    = (uint32_t)(nowMs - gLastMs);
  gLastMs = nowMs;

  bool fanOn = IsPowerOn;
  if (fanOn) gAccMs += dt;

  // Lưu NVS định kỳ
  if ((uint32_t)(nowMs - gLastSave) >= SAVE_INTERVAL_MS) {
    saveAccMs();
    gLastSave = nowMs;
  }

  

  // ===== Xử lý nút bật/tắt =====
  if (flagTogglePower) {
    IsPowerOn = !IsPowerOn;
    if (!IsPowerOn) {
      motor.stopMotor();
      sendMessage("{\"out1\": 0}");
      Serial.println("Động cơ đã TẮT");
    } else {
      Serial.println("Động cơ đã BẬT lại");
      motor.setMode(MODE_AUTO);
      lcd.updateMode(0);
      sendMessage("{\"out1\": 1}");
    }
    lcd.updatePowerState(IsPowerOn);
    lcd.updateDisplay();
    flagTogglePower = false;
  }

  // ===== Xử lý nút đổi chế độ =====
  if (flagChangeMode && IsPowerOn) {
    motor.nextMode();
    String s = String("{\"out2\": ") + String(motor.getMode()) + "}";
    sendMessage(s);
    Serial.print("Chuyển sang chế độ: ");
    Serial.println(modeToStr(motor.getMode()));
    lcd.updateMode(motor.getMode());
    flagChangeMode = false;
  }

  // ===== Đọc cảm biến và điều khiển tốc độ =====
  if (IsPowerOn) {
    sensor_mq135 = mq135_read();
    if (motor.getMode() == MODE_AUTO) {
      motor.setAutoSpeed(sensor_mq135.canhBao);
    }
    lcd.updateAQI(sensor_mq135.canhBao);
  }
  

 unsigned long now = millis();
  if (now - lastSendTime >= 5000) {
    lastSendTime = now;

    // Đọc nhiệt độ & độ ẩm
    sensor.readData();
    int temp = sensor.getTemperature();
    int hum = sensor.getHumidity();

    // Gửi nhiệt độ và độ ẩm lên MQTT
    char buffer[64];
    sprintf(buffer, "{\"temp\": %d, \"hum\": %d}", temp, hum);
    sendMessage2(buffer);

    Serial.print("Nhiệt độ TB: ");
    Serial.print(temp);
    Serial.println(" °C");

    Serial.print("Độ ẩm TB: ");
    Serial.print(hum);                                                                                                                                                                                                         
    Serial.println(" %");

    lcd.updateTempHum(temp, hum);

    // --- cập nhật dayLeft ---
    dayLeft = computeDayLeft();
    if (dayLeft != lastDayLeft) {
      lcd.updateDayLeft(dayLeft);
      lastDayLeft = dayLeft;
    }
  }
  
  delay(1000);
}