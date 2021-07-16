#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define WIFI_SSID "YOUR-SSID"
#define WIFI_PASSWORD "YOUR-PASSWORD"

#define UART_IN_BUFFER_LEN 1024

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <fieldset>
    <legend>Received</legend>
      %IN_BUFFER%
  </fieldset>
  <fieldset>
    <legend>Send</legend>
      <form action="/send">
        <input type="text" name="send">
        <input type="submit" value="Submit">
      </form>
  </fieldset>
</body></html>)rawliteral";

class UARTComm {
private:
  char _inBuffer[UART_IN_BUFFER_LEN];
  void (* _callback)(const char*, size_t length);
  char _endChar;
  char _in;

public:
  void begin(void (* callback)(const char*, size_t length), char endChar='$') {
    _callback = callback;
    _endChar = endChar;
    _clearBuffer();
  }

  static void sendMsg(const char *msg) {
    Serial.write(msg);
  }

  void tick() {
    while (Serial.available()) {
      _in = Serial.read();
      if (_in == _endChar) {
        _callback(_inBuffer, strlen(_inBuffer));
        _clearBuffer();
        while (Serial.available()) { Serial.read(); }
      } else if (_in == '\0') {
        // do nothing
      } else {
        if (strlen(_inBuffer) < UART_IN_BUFFER_LEN-1) {
          strcat(_inBuffer, &_in);
        }
      }
    }
  }

private:
  void _clearBuffer() {
    memset(_inBuffer, '\0', UART_IN_BUFFER_LEN);
  }
};


String webBuffer;

void cb(const char *in, size_t length) {
  UARTComm::sendMsg("received$");
  webBuffer = String(in);
}

UARTComm com;
AsyncWebServer server(80);

String processor(const String& var) {
  if (var == "IN_BUFFER") {
    return webBuffer;
  }
  return String();
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  digitalWrite(LED_BUILTIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  UARTComm::sendMsg("Connecting to wifi$");
  while (WiFi.status() != WL_CONNECTED) {
    UARTComm::sendMsg(".$");
    delay(250);  
  }
  UARTComm::sendMsg("done$");
  UARTComm::sendMsg("IP address:$");
  UARTComm::sendMsg(WiFi.localIP().toString().c_str()); UARTComm::sendMsg("$");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/send", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam("send")) {
      inputMessage = request->getParam("send")->value();
    }
    UARTComm::sendMsg(inputMessage.c_str());
    UARTComm::sendMsg("$");
    request->send(200, "text/html", "HTTP GET request sent to your ESP with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });

  server.begin();
  com.begin(&cb);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  com.tick();
  delay(10);
}
