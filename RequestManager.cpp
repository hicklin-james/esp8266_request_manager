#include "RequestManager.h"

/**
  Params:
    const char *ssid: Network ssid
    const char *password: Netork password
    SoftwareSerial/HardwareSerial: pointer to serial
  Information:
    Initializer for the RequestManager. Requires an ssid and password
    to connect to a network.
**/
#ifdef ESP_USE_SOFTWARE_SERIAL
  RequestManager::RequestManager(SoftwareSerial &ss) : _serial(&ss) {
    _serial->begin(9600);
  }
#else
  RequestManager::RequestManager(HardwareSerial &hs) : _serial(&hs) {
    _serial->begin(9600);
  }
#endif

void RequestManager::connect(const char *ssid, const char *password) {
  connectToNetwork(ssid, password);
}

bool RequestManager::isConnected() const {
  return _isConnected;
}

bool RequestManager::disconnect() {
  char disconnectCmd[10] = "AT+CWQAP";
  const char *p[] = {"OK"};
  bool r = sendCommand(disconnectCmd, NETWORK_CONNECTION_TIMEOUT, false, p, 1);
  _isConnected = r;
  return r;
}

/**
  Params:
    const char *ssid: Network ssid
    const char *password: Network password
  Returns:
    Bool: Whether connection to network was successful or not
  Information:
    An example command sent to the ESP8266 would look like this:
    AT+CWJAP="my-test-wifi","1234test"
    and the response would be:
    OK
**/
bool RequestManager::connectToNetwork(const char *ssid, const char *password) {
  char connectionCmd[128] = "";
  const char start[11] = "AT+CWJAP=\"";
  strcat(connectionCmd, start);
  strcat(connectionCmd, ssid);
  const char connector[4] = "\",\"";
  strcat(connectionCmd, connector);
  strcat(connectionCmd, password);
  const char secondConnector[2] = "\"";
  strcat(connectionCmd, secondConnector);
  const char *p[] = {"OK"};
  bool r = sendCommand(connectionCmd, NETWORK_CONNECTION_TIMEOUT, false, p, 1);
  _isConnected = r;
  return r;
}

/**
  Params:
    const char *host: The host url to make the request to (e.g. "jameshicklin.com")
    const char *req: The req uri (e.g. "/users/index")
    int: The port to connect to (e.g. 80)
  Returns:
    bool: A boolean indicating whether the request succeeded or not
          NOTE - a success is simply a request that was sent to the server.
          It says nothing about whether a response was received or not.
**/
bool RequestManager::get(const char *host, const char *req, int port) {
  _serial->flush();
  const char closeCommand[12] = "AT+CIPCLOSE";
  const char* p1[] = {"Linked", "ERROR", "we must restart", "OK"};
  char tcpConnector[128] {0};
  generateTcpConnectionString(tcpConnector, host, port);
  const char* p2[] = {"OK", "ALREADY CNNECTED"};
  bool tcpStatus = sendCommand(tcpConnector, TCP_CONNECT_TIMEOUT, false, p2, 2);
  if (tcpStatus) {
    char httpReq[256] {0};
    generateHttpReqString(httpReq, host, req);
    int reqLength = (int)strlen(httpReq);

    const char* p3[] = {"SEND OK"};
    bool reqStatus = sendRequest(httpReq, reqLength, p3, 1);
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());

    bool closeStatus = sendCommand(closeCommand, CLOSE_TCP_TIMEOUT, false, p1, 4);
    if (reqStatus && closeStatus) {
      return true;
    }
  }
  sendCommand(closeCommand, CLOSE_TCP_TIMEOUT, false, p1, 4);
  return false;
}

/**
  It isn't feasible to implement this function on an arduino with very limited memory at the time being.
  It might make sense to write out to a separate file instead of into a variable, but that has yet to be
  implemented.
**/
/**
void RequestManager::readResponseData(char *buf) {
  Serial.print("freeMemory()=");
  Serial.println(freeMemory());
  int max_length = HTTP_RESPONSE_INITIAL_SIZE;
  int data_index = 0;
  unsigned long start = millis();
  while ((millis() - start) < READ_RESPONSE_TIMEOUT) {
    while (_serial->available() > 0) {
      char c = _serial->read();
      buf[data_index] = c;
      //max_length = copyCharToStr(c, data, data_index, max_length);
      data_index++;
    }
  }
  //buf = data.c_str();
}
**/

/**
  Params:
    char *buf: pointer to a char array - assume that it is big enough to hold the command
    int reqLength: length of the request, as an integer
  Returns:
    void
  Information:
    This function fills the buf array with the AT+CIPSEND command that initiates the sending
    of data over the TCP link.
**/
void RequestManager::generateReqLenCommand(char *buf, int reqLength) {
  strcat(buf, "AT+CIPSEND=");
  String reqLengthStr(reqLength);
  strcat(buf, reqLengthStr.c_str());
}

/**
  Params:
    char *buf: pointer to a char array - assume that it is big enough to hold the command
    const char *host: pointer to a char array that corresponds to the host
    const char *req: pointer to a char array that corresponds to the request uri
  Returns:
    void
  Information:
    This function fills the buf array with the actual HTTP request content. See the HTTP/1.1
    specifications to see why it is formed the way it is.
**/
void RequestManager::generateHttpReqString(char *buf, const char *host, const char *req) {
  strcat(buf, "GET ");
  strcat(buf, req);
  const char httpHost[18] = " HTTP/1.1\r\nHost: ";
  strcat(buf, httpHost);
  strcat(buf, host);
  const char connUserAgent[50] = "\r\nConnection: keep-alive\r\nUser-Agent: Arduino\r\n\r\n";
  strcat(buf, connUserAgent);
}

/**
  Params:
    char *buf: pointer to a char array - assume that it is big enough to hold the command
    const char *host: pointer to a char array that corresponds to the host
    int port: int for the port that we should initiate the TCP link on - usually 80 (http) or 443 (https)
  Returns:
    void
  Information:
    This function fills the buf array with the AT+CIPSTART command that initiates the TCP link
**/
void RequestManager::generateTcpConnectionString(char *buf, const char *host, int port) {
  strcat(buf, "AT+CIPSTART=\"TCP\",\"");
  strcat(buf, host);
  const char blank[3] = "\",";
  strcat(buf, blank);
  String portStr(port);
  strcat(buf, portStr.c_str());
}

/**
  Params:
    const char *req: pointer to a char array that represents the request itself
    int reqLength: int representing the length of the request data to be send
    const char *successStrs[]: pointer to array of char arrays that represents the success strings to
                               wait for.
    int successStrsLength: length of the successStrs array parameter
  Returns:
    bool
  Information:
    This function executes an HTTP request. It assumes that a TCP link has already been opened. It first
    sends the AT+CIPSEND command (reqLenCommand), waits for a response from the serial line indicating it
    is ready to recieve data, and finally sends the actual data to be sent to the server.
**/
bool RequestManager::sendRequest(const char *req, int reqLength, const char *successStrs[], int successStrsLength) {
  // flush serial buffer
  _serial->flush();

  char reqLenCommand[32] {0};
  generateReqLenCommand(reqLenCommand, reqLength);

  _serial->println(reqLenCommand);

  unsigned long start;
  start = millis();
  bool found;
  while (millis()-start < GET_REQUEST_LENGTH_TIMEOUT) {                          
    if ( _serial->find(">") == true ) {
      found = true;
      break;
    }
  }
  if (found) {
    return sendCommand(req, GET_REQUEST_TIMEOUT, true, successStrs, successStrsLength);   
  }
  else {
    return false;
  }
}

/**
  Params:
    bool write: boolean indicating whether to use print or println
    const char *cmd: pointer to array holding command to be sent over serial
  Returns:
    void
  Information:
    This function is responsible for actually writing to the serial line
**/
void RequestManager::sendToSerial(bool write, const char *cmd) {
  if (write) {
    _serial->print(cmd);
  }
  else {
    _serial->println(cmd);
  }
}

/**
  Params:
    char c: character to copy
    char *str: pointer to dest array
    int index: index in str to copy to
    int maxLength: the maximum length of str
  Returns:
    int: an int representing the new maxLength
  Information:
    This function copies c into str, at index. It dynamically increases the size of str if
    neccessary
**/
int RequestManager::copyCharToStr(char c, char *str, int index, int maxLength) {
  if (index < maxLength) {
    str[index] = c;
    return maxLength;
  }
  else {
    realloc(str, (maxLength*2) + 1); //new char[(maxLength * 2) + 1]();
    for (int i = maxLength; i <= (maxLength * 2); i++) {
      str[i] = '\0';
    }
    str[index] = c;
    return maxLength * 2;
  }
}

/**
  Params:
    const char *cmd: pointer to a char array that represents the command to be send
    int timeout: int representing the amount of time to wait for a success string
    bool write: bool indicating whether to use print or println
    const char *successStrs[]: pointer to array of char arrays that represents the success strings to
                               wait for.
    int successStrsLength: length of the successStrs array parameter
  Returns:
    bool
  Information:
    This function is responsible for sending a generic command over the serial line. It waits for any string in
    successStrs, and then returns a boolean indicating whether a success string was found or not.
**/
bool RequestManager::sendCommand(const char *cmd, int timeout, bool write, const char *successStrs[], int successStrsLength) {
  _serial->flush();
  sendToSerial(write, cmd);

  unsigned long start = millis();
  char *data = (char*)malloc((HTTP_RESPONSE_INITIAL_SIZE+1) * sizeof(char*)); //char[HTTP_RESPONSE_INITIAL_SIZE+1]();
  for (int i = 0; i <= HTTP_RESPONSE_INITIAL_SIZE; i++) {
    data[i] = '\0';
  }
  int max_length = HTTP_RESPONSE_INITIAL_SIZE; 
  int data_index = 0;
  bool successFlag = false;
  while ((millis() - start) < timeout) {
    while (_serial->available() > 0) {
      char c = _serial->read();
      max_length = copyCharToStr(c, data, data_index, max_length);
      data_index++;
    }

    for (int i = 0; i < successStrsLength; i++) {
      const char *successStr = successStrs[i];
      //if (data.indexOf(successStr) > -1) {
      if (strstr(data, successStr)) {
        successFlag = true;
        break;
      }
    }
    if (successFlag) {
      break;
    }
  }
  free(data);
  return successFlag;
}
