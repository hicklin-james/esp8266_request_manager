#ifndef __REQUEST_MANAGER_H_INCLUDED__
#define __REQUEST_MANAGER_H_INCLUDED__

#include <Arduino.h>
#include <stdarg.h>
#include <SoftwareSerial.h>

#define TCP_CONNECT_TIMEOUT 4000
#define GET_REQUEST_LENGTH_TIMEOUT 1000
#define GET_REQUEST_TIMEOUT 10000
#define NETWORK_CONNECTION_TIMEOUT 8000
#define CLOSE_TCP_TIMEOUT 1000
#define READ_RESPONSE_TIMEOUT 6000

#define HTTP_RESPONSE_INITIAL_SIZE 64

// uncomment this to use SoftwareSerial
#define ESP_USE_SOFTWARE_SERIAL

#define BAUD_RATE 9600

struct CommandAttrs {
  char *response;
  bool success;
};

class RequestManager {

  public:
    #ifdef ESP_USE_SOFTWARE_SERIAL
      RequestManager( SoftwareSerial &ss );
    #else
      RequestManager( HardwareSerial &ss );
    #endif

    void connect(const char *ssid, const char *password);
    bool isConnected() const;
    bool get(const char *host, const char *req, int port);
    bool post(const char *host, const char *req, const char *body, int port);

  private:
    bool connectToNetwork(const char *ssid, const char *password);
    bool disconnect();
    bool sendCommand(const char *cmd, int timeout, bool write, const char* successStrs[], int successStrsLength);
    bool sendRequest(const char *req, int reqLength, const char *successStrs[], int successStrsLength);
    void generateTcpConnectionString(char *buf, const char *host, int port);
    void generateHttpGetReqString(char *buf, const char *host, const char *req);
    void generateHttpPostReqString(char *buf, const char *host, const char *req, const char *body);
    void generateReqLenCommand(char *buf, int reqLength);
    void sendToSerial(bool write, const char *cmd);
    int copyCharToStr(char c, char *str, int index, int maxLength);
    //void readResponseData(char *buf);
    
    bool _isConnected;

    #ifdef ESP_USE_SOFTWARE_SERIAL
      SoftwareSerial *_serial;
    #else
      HardwareSerial *_serial;
    #endif
};

#endif
