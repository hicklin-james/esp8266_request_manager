# esp8266_request_manager
Arduino request manager for the esp8266

This library currently only supports GET requests. My use case for this was to send data to a server using a query string - I didn't care about the response. So there is no parsing of the response body involved. It just makes the request to the server, and that is that.

###Important
Remember to set the baud rate of the ESP8266 module to the same baud rate that is used in the library. The library defaults to a baud rate of 38400. To set the baud rate on the ESP8266, issue the following command: `AT+CIOBAUD=38400` and then restart the device.

Example use:

```
#include <SoftwareSerial.h>
#include <RequestManager.h>

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

###To Use SoftwareSerial
Uncomment `#define ESP_USE_SOFTWARE_SERIAL` in RequestManager.h
