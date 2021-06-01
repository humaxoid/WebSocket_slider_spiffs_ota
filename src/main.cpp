// Импортируем библиотеки
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>
 
// Замените на свой учетные данные сети
const char* ssid = "***";
const char* password = "******";
 
// Создаем сервер через 80 порт
AsyncWebServer server(80);
 
// Создаем объект WebSocket
AsyncWebSocket ws("/ws");
 
// Указываем количество выходов
#define NUM_OUTPUTS  5
 
// Присваиваем каждому GPIO свой выход
int outputGPIOs[NUM_OUTPUTS] = {32, 33, 25, 26, 27};
 
// Запускаем SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}
 
// Запускаем WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
 
String getOutputStates(){
  JSONVar myArray;
  for (int i =0; i<NUM_OUTPUTS; i++){
    myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
    myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
  }
  String jsonString = JSON.stringify(myArray);
  return jsonString;
}
 
void notifyClients(String state) {
  ws.textAll(state);
}
 
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "states") == 0) {
      notifyClients(getOutputStates());
    }
    else{
      int gpio = atoi((char*)data);
      digitalWrite(gpio, !digitalRead(gpio));
      notifyClients(getOutputStates());
    }
  }
}
 
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,AwsEventType type,
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
 
void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}
 
void setup(){
  // Монитор порта для дебага
  Serial.begin(115200);
 
  // Назначаем GPIO выходами
  for (int i =0; i<NUM_OUTPUTS; i++){
    pinMode(outputGPIOs[i], OUTPUT);
  }
  initSPIFFS();
  initWiFi();
  initWebSocket();
 
  // Начальная страница
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html",false);
  });
 
  server.serveStatic("/", SPIFFS, "/");
 
  // Запускаем ElegantOTA
  AsyncElegantOTA.begin(&server);
  
  // Запускаем сервер
  server.begin();
}
 
void loop() {
  AsyncElegantOTA.loop();
  ws.cleanupClients();
}