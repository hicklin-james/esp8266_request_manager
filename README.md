# ESP8266 Request Manager
## Arduino request manager for the esp8266

This is a library for using the ESP 8266 Wifi chip to make HTTP requests to a remote server. It uses the default `AT` commands that come with the chip. It supports connections from Arduino to ESP 8266 via `HardwareSerial` or `SoftwareSerial`.

The library currently only supports `GET` and `POST` requests. My use case for this was to send data from an arduino to a remote server using an HTTP request and query string. I didn't care about the response, so there is no parsing of the response body involved. It just makes the request to the server, and that is that.

### Important
Remember to set the baud rate of the ESP8266 module to the same baud rate that is used in the library. The library defaults to a baud rate of `9600`. To set the baud rate on the ESP8266, issue the following command: `AT+CIOBAUD=38400` and then restart the device.

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
  // get request
  reqManager.get("my_url.com", "/route/to/get/action?param1=value1&param2=value2", 80);
  // post request
  reqManager.post("my_url.com", "/route/to/post/action", "param1=value1&param2=value2", 80);
  delay(10000);
}
```

### To Use SoftwareSerial
Uncomment `#define ESP_USE_SOFTWARE_SERIAL` in RequestManager.h
