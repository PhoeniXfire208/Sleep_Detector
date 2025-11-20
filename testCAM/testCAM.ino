#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Настройки камеры (конфигурация для AI-Thinker ESP32-CAM)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Настройки Wi-Fi (должны совпадать с настройками NodeMCU)
const char* ssid = "PhoeniXfire";
const char* password = "34553455";

// IP-адрес вашей NodeMCU (ЗАМЕНИТЕ на адрес, который был в мониторе порта NodeMCU)
const String nodeMCU_IP = "192.168.233.153";

// Переменные для логики детектирования
bool alarmState = false;
unsigned long alarmStartTime = 0;
const unsigned long ALARM_DELAY = 3000; // Тревога через 3 секунды закрытых глаз

void setup() {
  Serial.begin(115200);

  // Настройка и инициализация камеры
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  // ... остальные пины конфигурации камеры (из предыдущего кода)
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Ошибка инициализации камеры: 0x%x", err);
    return;
  }

  // Подключение к Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключаемся к WiFi...");
  }
  Serial.println("WiFi подключен!");
  Serial.print("IP адрес ESP32-CAM: ");
  Serial.println(WiFi.localIP());
}

void sendAlarmCommand(bool alarmOn) {
  HTTPClient http;
  String url = "http://" + nodeMCU_IP + (alarmOn ? "/ALARM_ON" : "/ALARM_OFF");
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    Serial.println(alarmOn ? "Команда ALARM_ON отправлена" : "Команда ALARM_OFF отправлена");
  } else {
    Serial.print("Ошибка отправки команды: ");
    Serial.println(httpCode);
  }
  http.end();
}

bool detectClosedEyes() {
  // ЗАГЛУШКА: Вставьте сюда ваш алгоритм машинного зрения
  // Пока можно сэмулировать детектирование для теста
  // Например, отправить '1' в монитор порта для имитации закрытых глаз
  
  if (Serial.available()) {
    char command = Serial.read();
    if (command == '1') {
      return true;
    }
  }
  return false;
}

void loop() {
  // Проверяем, обнаружены ли закрытые глаза
  if (detectClosedEyes()) {
    if (!alarmState) {
      // Первое обнаружение закрытых глаз
      alarmStartTime = millis();
      alarmState = true;
      Serial.println("Закрытые глаза обнаружены. Ожидание...");
    } else if (millis() - alarmStartTime > ALARM_DELAY) {
      // Глаза закрыты дольше порогового времени
      sendAlarmCommand(true);
      alarmState = false; // Сбрасываем состояние после включения тревоги
    }
  } else {
    // Глаза открыты
    if (alarmState) {
      // Сбрасываем таймер, если глаза открылись до срабатывания тревоги
      alarmState = false;
      Serial.println("Глаза открыты. Тревога отменена.");
    }
    // Всегда выключаем сигнализацию, когда глаза открыты
    sendAlarmCommand(false);
  }

  delay(100); // Небольшая задержка между проверками
}
