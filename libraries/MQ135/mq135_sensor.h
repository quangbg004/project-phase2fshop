#ifndef MQ135_SENSOR_H
#define MQ135_SENSOR_H

#include <Arduino.h>
#include <MQUnifiedsensor.h>

struct MQ135_Data {
  float CO;
  float Alcohol;
  float CO2;
  float Toluen;
  float NH4;
  float Aceton;
  int ADC;
  float Voltage;
  float RS;
  float R0;
  float Ratio;
  int canhBao;
};

void mq135_init();
MQ135_Data mq135_read();
void mq135_print(const MQ135_Data& data);

#endif
