#include "mq135_sensor.h"

#define Board "ESP-32"
#define Voltage_Resolution 3.3
#define ADC_Bit_Resolution 12
#define pin 34
#define type "MQ-135"  
#define RatioMQ135CleanAir 3.6

MQUnifiedsensor mq135(Board, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

enum MucCanhBao {TOT = 0, AN_TOAN, TRUNG_BINH, KEM, NGUY_HIEM};

static MucCanhBao xepLoai(float giaTri, float n1, float n2, float n3, float n4) {
  if (giaTri < n1) return TOT;
  else if (giaTri < n2) return AN_TOAN;
  else if (giaTri < n3) return TRUNG_BINH;
  else if (giaTri < n4) return KEM;
  else return NGUY_HIEM;
}

static String tenMuc(MucCanhBao muc) {
  switch (muc) {
    case TOT: return "Tốt ✅";
    case AN_TOAN: return "An toàn 👍";
    case TRUNG_BINH: return "Trung bình ⚠️";
    case KEM: return "Kém ❗";
    case NGUY_HIEM: return "Nguy hiểm 🚨";
    default: return "";
  }
}

void mq135_init() {
  mq135.setRegressionMethod(1);
  mq135.init();

   mq135.setR0(25.0); // kOhm - R0 cố địnhyt        bbb

}

MQ135_Data mq135_read() {
  MQ135_Data data;
  float cf = 1.0;

  mq135.update();

  mq135.setA(605.18); mq135.setB(-3.937);
  data.CO = mq135.readSensor();

  mq135.setA(77.255); mq135.setB(-3.18);
  data.Alcohol = mq135.readSensor(false, cf);

  mq135.setA(110.47); mq135.setB(-2.862);
  data.CO2 = mq135.readSensor(false, cf) + 400;

  mq135.setA(44.947); mq135.setB(-3.445);
  data.Toluen = mq135.readSensor(false, cf);

  mq135.setA(102.2); mq135.setB(-2.473);
  data.NH4 = mq135.readSensor(false, cf);

  mq135.setA(34.668); mq135.setB(-3.369);
  data.Aceton = mq135.readSensor(false, cf);

  data.ADC = analogRead(pin);
  data.Voltage = mq135.getVoltage();
  data.RS = mq135.getRS();
  data.R0 = mq135.getR0();
  data.Ratio = mq135.getRS() / mq135.getR0();

  // Xếp loại
  MucCanhBao mucMax = TOT;
  mucMax = max(mucMax, xepLoai(data.CO, 4, 9, 35, 100));
  mucMax = max(mucMax, xepLoai(data.Alcohol, 10, 50, 200, 500));
  mucMax = max(mucMax, xepLoai(data.CO2, 400, 1000, 2000, 5000));
  mucMax = max(mucMax, xepLoai(data.Toluen, 50, 100, 300, 600));
  mucMax = max(mucMax, xepLoai(data.NH4, 25, 50, 100, 300));
  mucMax = max(mucMax, xepLoai(data.Aceton, 50, 100, 300, 600));

  data.canhBao = mucMax;

  return data;
}

void mq135_print(const MQ135_Data& d) {
  Serial.println("====== Đọc dữ liệu từ MQ135 ======");
  Serial.print("ADC: "); Serial.println(d.ADC);
  Serial.print("Điện áp (V): "); Serial.println(d.Voltage);
  Serial.print("RS: "); Serial.println(d.RS);
  Serial.print("R0: "); Serial.println(d.R0);
  Serial.print("RS/R0: "); Serial.println(d.Ratio);
  Serial.print("CO (ppm): "); Serial.println(d.CO);
  Serial.print("Alcohol (ppm): "); Serial.println(d.Alcohol);
  Serial.print("CO2 (ppm): "); Serial.println(d.CO2);
  Serial.print("Toluen (ppm): "); Serial.println(d.Toluen);
  Serial.print("NH4 (ppm): "); Serial.println(d.NH4);
  Serial.print("Aceton (ppm): "); Serial.println(d.Aceton);
  Serial.print("→ Cảnh báo: "); Serial.println(d.canhBao);
  Serial.println("===================================");
}