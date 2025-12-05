#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// Настройки вашей Wi-Fi сети
const char* ssid = "PhoeniXfire";
const char* password = "34553455";

// Создаем веб-сервер на 80-м порту
ESP8266WebServer server(80);

// Пины для подключения устройств
const int ledPin = 5;     // D1 для светодиода
const int buzzerPin = 4;  // D2 для зуммера

void setup() {
  Serial.begin(115200);
  
  // Настраиваем пины как выходы
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  // Гарантированно выключаем все при старте
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, HIGH); // Зуммер выключен при HIGH (инверсная логика)
  
  // Подключаемся к Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Подключаемся к WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi подключен!");
  Serial.print("IP адрес NodeMCU: ");
  Serial.println(WiFi.localIP()); // Запишите этот адрес, он понадобится для ESP32-CAM

  // Настраиваем обработчики веб-запросов
  server.on("/ALARM_ON", HTTP_GET, []() {
    digitalWrite(ledPin, HIGH);      // Включить светодиод
    digitalWrite(buzzerPin, LOW);    // Включить зуммер (LOW для инверсной логики)
    Serial.println("ТРЕВОГА: Светодиод и зуммер ВКЛЮЧЕНЫ");
    server.send(200, "text/plain", "ALARM ON");
  });

  server.on("/ALARM_OFF", HTTP_GET, []() {
    digitalWrite(ledPin, LOW);       // Выключить светодиод
    digitalWrite(buzzerPin, HIGH);   // Выключить зуммер
    Serial.println("ТРЕВОГА: Светодиод и зуммер ВЫКЛЮЧЕНЫ");
    server.send(200, "text/plain", "ALARM OFF");
  });

  // Запускаем сервер
  server.begin();
  Serial.println("HTTP сервер запущен");
}

void loop() {
  server.handleClient(); // Постоянно обрабатываем входящие запросы
}
