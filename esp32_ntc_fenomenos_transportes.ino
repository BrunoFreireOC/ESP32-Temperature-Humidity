#include <WiFi.h>
#include <WebServer.h>

#include "User_lib.hpp"

/* Private Defines Begin */
#define LED_BUILTIN 2

float NTC_Temperatures[NUM_NTC];

bool flag_WIFI_State = false;

const uint16_t interval = 1000; // 1 Segundo
uint16_t previousMillis = 0;

void setup() {
  Serial.begin(115200);
  EEPROM_Init();

  pinMode(LED_BUILTIN, OUTPUT);

  loadCredentials();  // Carrega SSID/senha da EEPROM
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

  uint8_t timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout <= 15) {
    delay(1000);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    startAPMode();  // Falha? Ativa o modo AP
  } else {
    for (uint8_t index = 0; index <= 15; index++){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("\nConectado ao WiFi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    flag_WIFI_State = true;

    Server_Init();
  }
  DHT_Init();

}

void loop() {

  if (flag_WIFI_State == true){
    
    for (uint8_t index = 0; index < NUM_NTC; index++){
      read_NTC_Temperature(index);
      NTC_Temperatures[index] = ntc[index].Temp;
      //NTC_Temperatures[index] = read_NTC_Temperature(index);
      addToNTCBuffer(index, NTC_Temperatures[index]);
      tempNTC_media = getNTCAverage(index);
    }
    

    digitalWrite(LED_BUILTIN, HIGH);
    Server_HandleClient();
    delay(30);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  } else {
    Server_HandleClient();
  }

  unsigned long currentMillis = millis();

  if ( (currentMillis - previousMillis >= interval) && (flag_WIFI_State == true)) {
    previousMillis = currentMillis;

    dht1_temp = DHT_Temp_Measure(1);
    dht1_humi = DHT_Humidity_Measure(1);
    dht2_temp = DHT_Temp_Measure(2);
    dht2_humi = DHT_Humidity_Measure(2);
  }

  if (loggingActive && millis() - lastLogTime >= 1000) {
    lastLogTime = millis();
    csvBuffer += String(millis());

    for (int i = 0; i < NUM_NTC; i++) {
      csvBuffer += "," + String(ntc[i].Temp, 2);
    }

    csvBuffer += "," + String(dht1_temp, 2) + "," + String(dht1_humi, 2);
    csvBuffer += "," + String(dht2_temp, 2) + "," + String(dht2_humi, 2);
    csvBuffer += "\n";
  }


  
  
}


