# ESP32-Temperature-Humidity

Monitoring up to 6 NTC thermistors and 2 DHTs for data acquisition. The ESP-WROOM-32 sends the data via WiFi, that can also be recorded to an .csv file.

If no saved WiFi netword is detected, the ESP32 will create a Access Point (AP), which the user can connect and set a new WiFi SSID and Password. You can tell if that's the case when the LED onboard is continuous. When that's done, the ESP32 will reset and try to connect with the new configurations.

When connected to the WiFi network, the ESP32 will create a HTTP server on the network and starts sending data. The builtin LED will blink every time a new data is sent.

The OTA updates is available, the user can send a .bin file of the software to update via WiFi.

The HTML page supports all the monitoring data and .csv file recording, as well as OTA update.

This code was designed to "Transport Phenomena" (Fenômenos dos Transportes) subject.
