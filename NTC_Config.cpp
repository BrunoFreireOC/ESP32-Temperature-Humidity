#include "User_lib.hpp"
#include <DHT.h>

float buffer[NUM_NTC][BUFFER_SIZE];  // [sensor][leituras]
int bufferIndex[NUM_NTC] = {0};      // Índice circular de cada NTC
int bufferCount[NUM_NTC] = {0};      // Número de amostras válidas
float tempNTC_media = 0;

/* Private Variables Begin */

NTC ntc[NUM_NTC] = {
  {NTC_0, 10000.0, 3950.0, 0},
  // {NTC_1, 10000.0, 3950.0, 0},
  {NTC_2, 10000.0, 3950.0, 0},
  {NTC_3, 10000.0, 3950.0, 0},
  {NTC_4, 10000.0, 3950.0, 0},
  {NTC_5, 10000.0, 3950.0, 0},
  // {NTC_6, 10000.0, 3950.0, 0},
};

DHT dht1(DHT1_PIN, DHTTYPE);
DHT dht2(DHT2_PIN, DHTTYPE);

float dht1_temp = 0.0, dht1_humi = 0.0;
float dht2_temp = 0.0, dht2_humi = 0.0;

float read_NTC_Temperature(uint8_t ntcIndex) {
  // Convert binary ADC to voltage value. Remember that ESP32 have 3.3V reference voltage for ADC conversions.
  float VoltageNTC = (float)analogRead(ntc[ntcIndex].NTC_GPIO) * 3.3f / 4095.0f;

  // Convert voltage in NTC to Resistance, considering a 10k resistor divider and NTC is next to GND. (3V3 -> 10kR - NTC -> GND)
  float Rntc = (VoltageNTC * 10000.0)/(3.3 - VoltageNTC);
  ntc[ntcIndex].Rcalc = Rntc;

  // Based on NTC resistance and other values gathered before, calculate the temperatura based on Stenhart-Hart equation
  float T0 = 25.0 + 273.15; // 25°C in Kelvin
  float beta = ntc[ntcIndex].beta;
  float R0 = ntc[ntcIndex].R0;
  float T = 1.0 / (1.0/T0 + (1.0/beta) * log(Rntc/R0)); // Temperature in Kelvin
  ntc[ntcIndex].Temp = T - 273.15f;
  return T - 273.15; // Converte para Celsius
}

void addToNTCBuffer(uint8_t ntcIndex, float value) {
  buffer[ntcIndex][bufferIndex[ntcIndex]] = value;

  bufferIndex[ntcIndex] = (bufferIndex[ntcIndex] + 1) % BUFFER_SIZE;

  if (bufferCount[ntcIndex] < BUFFER_SIZE)
    bufferCount[ntcIndex]++;
}

float getNTCAverage(uint8_t ntcIndex) {
  float sum = 0.0;
  for (int i = 0; i < bufferCount[ntcIndex]; i++) {
    sum += buffer[ntcIndex][i];
  }
  return bufferCount[ntcIndex] > 0 ? sum / bufferCount[ntcIndex] : 0.0;
}

void DHT_Init(){
  dht1.begin();
  dht2.begin();
}

float DHT_Temp_Measure(uint8_t dht_select){
  if (dht_select == 1){
    dht1_temp = dht1.readTemperature();
    if (isnan(dht1_temp)){
      return 0;
    }
    return dht1_temp;
  } else {
    dht2_temp = dht2.readTemperature();
    if (isnan(dht2_temp)){
      return 0;
    }
    return dht2_temp;
  }
}

float DHT_Humidity_Measure(uint8_t dht_select){
  if (dht_select == 1){
    dht1_humi = dht1.readHumidity();
    if (isnan(dht1_humi)){
      return 0;
    }
    return dht1_humi;
  } else {
    dht2_humi = dht2.readHumidity();
    if (isnan(dht2_humi)){
      return 0;
    }
    return dht2_humi;
  }
}

