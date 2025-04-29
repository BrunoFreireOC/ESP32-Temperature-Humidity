#ifndef USER_LIB_HPP
#define USER_LIB_HPP

#include "User_define.hpp"
#include "Arduino.h"

extern float buffer[NUM_NTC][BUFFER_SIZE];  // [sensor][leituras]
extern int bufferIndex[NUM_NTC];      // Índice circular de cada NTC
extern int bufferCount[NUM_NTC];      // Número de amostras válidas
extern float tempNTC_media;

struct NTC {
  uint8_t NTC_GPIO; // Numero do GPIO do NTC
  float R0;   // Resistência a 25°C (ex.: 10000.0)
  float beta; // Beta ajustado (ex.: 3950.0)
  float Rcalc;
  float Temp;
};

extern NTC ntc[NUM_NTC];

float read_NTC_Temperature(uint8_t ntcIndex);
void addToNTCBuffer(uint8_t ntcIndex, float value);
float getNTCAverage(uint8_t ntcIndex);

extern float dht1_temp, dht1_humi;
extern float dht2_temp, dht2_humi;

float DHT_Temp_Measure(uint8_t dht_select);
float DHT_Humidity_Measure(uint8_t dht_select);

/* =================== */
/* SERVER CONFIG BEGIN */
/* =================== */

extern bool loggingActive;
extern String csvBuffer;
extern String currentCSVFileName;
extern unsigned long lastLogTime;

extern const char* AP_SSID;
extern const char* AP_PASS;

extern String savedSSID;
extern String savedPassword;

void DHT_Init();
void Server_Init();
void Server_HandleClient();

void handleRoot();
void handleData();
void startAPMode();
void handleRootAP();
void handleConfigAP();
void handleNotFound();

void EEPROM_Init();
void saveCredentials(String ssid, String password);
void loadCredentials();

#endif