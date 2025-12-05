#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Конфигурация камеры для ESP32-CAM
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
#define CAMERA_MODEL_AI_THINKER

// Настройки Wi-Fi
const char* ssid = "PhoeniXfire";
const char* password = "34553455";

// IP адрес NodeMCU (ЗАМЕНИТЕ на реальный!)
const String nodeMCU_IP = "192.168.233.153";

// Переменные для видеопотока
camera_fb_t * fb = NULL;

// Функция для запуска streaming сервера
void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
  // Настройка камеры
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Начальное разрешение можно поставить меньше для экономии памяти
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // UXGA - 1600x1200
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA; // SVGA - 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Ошибка инициализации камеры: 0x%x", err);
    return;
  }
  Serial.println("✅ Камера инициализирована");

  // Настройка параметров камеры (опционально)
  sensor_t * s = esp_camera_sensor_get();
  // Настройки для лучшего качества при разных условиях
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

  // Подключение к Wi-Fi
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  
  Serial.print("📡 Подключаемся к WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi подключен!");
  Serial.print("🌐 IP адрес: ");
  Serial.println(WiFi.localIP());

  // ⭐⭐ ВАЖНО: ЗАПУСК СЕРВЕРА ДЛЯ ВИДЕОПОТОКА ⭐⭐
  startCameraServer();
  
  Serial.println("🎥 Камера готова!");
  Serial.print("📹 Streaming URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/stream");
  Serial.println("📸 Фото URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/capture");
}

void sendAlarmCommand(bool alarmOn) {
  HTTPClient http;
  String url = "http://" + nodeMCU_IP + (alarmOn ? "/ALARM_ON" : "/ALARM_OFF");
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    Serial.println(alarmOn ? "🚨 ТРЕВОГА: Сигнализация ВКЛЮЧЕНА" : "✅ Сигнализация ВЫКЛЮЧЕНА");
  } else {
    Serial.print("❌ Ошибка отправки команды: ");
    Serial.println(httpCode);
  }
  http.end();
}

void loop() {
  // Основная логика детектирования может быть здесь
  // Но для машинного обучения мы будем использовать Python
  
  delay(1000);
  
  // Простая проверка связи (можно удалить)
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    Serial.println("📊 Система работает...");
    lastCheck = millis();
  }
}

// ⭐⭐ ДОБАВЬТЕ ЭТИ БИБЛИОТЕКИ В ARDUINO IDE ⭐⭐
// Для работы streaming сервера нужны дополнительные библиотеки:
// 1. ESPAsyncWebServer
// 2. AsyncTCP
//
// Установите их через Менеджер библиотек:
// - Скетч -> Подключить библиотеку -> Управлять библиотеками
// - Найдите "ESPAsyncWebServer" и установите
// - Найдите "AsyncTCP" и установите
