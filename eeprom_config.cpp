#include "User_lib.hpp"
#include <EEPROM.h>  // Save SSID and Password credentials

void EEPROM_Init(){
  EEPROM.begin(512);
}

void saveCredentials(String ssid, String password) {
  EEPROM.writeString(0, ssid);
  EEPROM.writeString(100, password);  // Offset 100 para evitar sobreposição
  EEPROM.commit();
}

void loadCredentials() {
  savedSSID = EEPROM.readString(0);
  savedPassword = EEPROM.readString(100);
}
