# esp8266_request_manager
Arduino request manager for the esp8266

This library currently only supports get requests.

###Important
Remember to set the baud rate of the ESP8266 module to the same baud rate that is used in the library. The library defaults to a baud rate of 38400. To set the baud rate on the ESP8266, issue the following command: `AT+CIOBAUD=38400` and then restart the device.

Example use:

```
#include <SoftwareSerial.h>
#include <RequestManager.h>
#include <avr/pgmspace.h>
#include <MemoryFree.h>

const char ssid[8] = "my_ssid";
const char password[14] = "my_wifi_password";

SoftwareSerial ss(10,11);
RequestManager reqManager(ss);

void setup() {
  reqManager.connect(ssid, password);
}

void loop() {
  reqManager.get("google.com", "/", 80);
  delay(10000);
}
```
