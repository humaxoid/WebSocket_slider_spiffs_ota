// Import required libraries for ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Network settings
const char* ssid = "uname";
const char* password = "pass";
IPAddress local_IP(192, 168, 1, 6);    // Задаем статический IP-адрес:
IPAddress gateway(192, 168, 1, 1);     // Задаем IP-адрес сетевого шлюза:
IPAddress subnet(255, 255, 255, 0);    // Задаем маску сети:
IPAddress primaryDNS(8, 8, 8, 8);      // Основной ДНС (опционально)
IPAddress secondaryDNS(8, 8, 4, 4);    // Резервный ДНС (опционально)

bool ledState1 = 0;
bool ledState2 = 0;
bool ledState3 = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Notify clients about the current status of the LED
void notifyClients1() {ws.textAll(String(ledState1));}
void notifyClients2() {ws.textAll(String(ledState2+2));}
void notifyClients3() {ws.textAll(String(ledState3+4));}
  
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
      if (strcmp((char*)data, "toggle1") == 0) {ledState1 = !ledState1; notifyClients1();} 
      if (strcmp((char*)data, "toggle2") == 0) {ledState2 = !ledState2; notifyClients2();}
	  if (strcmp((char*)data, "toggle3") == 0) {ledState3 = !ledState3; notifyClients3();}
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:                 
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:              
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:                    
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:                    
    case WS_EVT_ERROR:                   
      break;
  }
}

// Инициализация WebSocket
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var) {
  Serial.println(var);
  if (var == "STATE1") {if (ledState1) {return "ON";}else {return "OFF";}}
  if (var == "STATE2") {if (ledState2) {return "ON";}else {return "OFF";}}
  if (var == "STATE3") {if (ledState3) {return "ON";}else {return "OFF";}}
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(32, OUTPUT); digitalWrite(32, LOW);
  pinMode(33, OUTPUT); digitalWrite(33, LOW);
  pinMode(25, OUTPUT); digitalWrite(25, LOW);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Client mode could not be configured");
  }

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

 // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
   request->send(SPIFFS, "/script.js", "text/javascript");
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  digitalWrite(32, ledState1);
  digitalWrite(33, ledState2);
  digitalWrite(25, ledState3);
}