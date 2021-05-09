// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Задаем сетевые настройки
const char* ssid = "Uname";
const char* password = "pass";
IPAddress local_IP(192, 168, 11, 68);
IPAddress gateway(192, 168, 11, 102);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4); 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool ledState1 = 0;
bool ledState2 = 0;

// Notify customers about the current status of the LED
void notifyClients1() {
  ws.textAll(String(ledState1));
}

void notifyClients2() {
  ws.textAll(String(ledState2));
}

/* функция обратного вызова, которая запускается всякий раз, когда мы получаем новые
  данные от клиентов по протоколу WebSocket. Если мы получаем сообщение “toggle”, мы
  переключаем значение переменной ledState. Кроме того, мы уведомляем всех клиентов,
  вызывая функцию notifyClients (). Таким образом, все клиенты получают уведомление об
  изменении и соответствующим образом обновляют интерфейс.*/
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle1") == 0) {
      ledState1 = !ledState1;
      notifyClients1();
    }
    if (strcmp((char*)data, "toggle2") == 0) {
      ledState2 = !ledState2;
      notifyClients2();
    }
  }
}


// Настройка прослушивателя событий для обработки различных асинхронных шагов протокола WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:                  // когда клиент вошел в систему
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:               // когда клиент вышел из системы
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:                     // при получении пакета данных от клиента
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:                     // в ответ на запрос ping
    case WS_EVT_ERROR:                    // при получении ошибки от клиента
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
  if (var == "STATE1") {
    if (ledState1) {
      return "ON";
    }
    else {
      return "OFF";
    }
  }

  if (var == "STATE2") {
    if (ledState2) {
      return "ON";
    }
    else {
      return "OFF";
    }
  }
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(32, OUTPUT);
  digitalWrite(32, LOW);
  pinMode(33, OUTPUT);
  digitalWrite(33, LOW);

  // Настраиваем статический IP-адрес:
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Режим клиента не удалось настроить");
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
}
