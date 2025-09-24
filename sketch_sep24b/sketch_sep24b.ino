#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>

// Настройка камеры - конфиг для AI-Thinker ESP32-CAM
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Настройки Wi-Fi (должны совпадать с NodeMCU)
const char* ssid = "ВАШ_WIFI_SSID";
const char* password = "ВАШ_WIFI_PASSWORD";

// MQTT настройки
const char* mqtt_server = "test.mosquitto.org";
const char* topic = "driver/drowsiness";

WiFiClient espClient;
PubSubClient client(espClient);

// Переменные для детектирования
unsigned long lastDetection = 0;
int eyesClosedCounter = 0;
const int EYES_CLOSED_THRESHOLD = 10; // Количество кадров с закрытыми глазами до тревоги

void setup_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d4 = Y7_GPIO_NUM;
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
  config.pixel_format = PIXFORMAT_GRAYSCALE; // Градации серого для скорости
  config.frame_size = FRAMESIZE_QQVGA;       // 160x120 - низкое разрешение для скорости
  config.jpeg_quality = 10;
  config.fb_count = 1;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Ошибка инициализации камеры 0x%x", err);
    return;
  }
}

// Упрощенная функция детектирования закрытых глаз (заглушка)
// В реальном проекте здесь будет сложный алгоритм с использованием OpenCV
bool detectClosedEyes(camera_fb_t* fb) {
  // ЗАГЛУШКА: Здесь должен быть ваш алгоритм детектирования глаз
  // Пока просто возвращаем случайное значение для теста
  // В реальности: анализ изображения, поиск глаз, расчет EAR и т.д.
  
  // Для тестирования: каждые 50 кадров "обнаруживаем" закрытые глаза
  static int frameCount = 0;
  frameCount++;
  
  if (frameCount % 50 == 0) {
    Serial.println("Обнаружены закрытые глаза! (тест)");
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  
  // Подключение к Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключение к WiFi...");
  }
  Serial.println("WiFi подключен");
  
  // Инициализация камеры
  setup_camera();
  Serial.println("Камера инициализирована");
  
  // Настройка MQTT
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    // Переподключение к MQTT
    if (client.connect("ESP32-CAM-Driver")) {
      Serial.println("MQTT подключен");
    }
  }
  client.loop();
  
  // Захват кадра
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Ошибка захвата кадра");
    return;
  }
  
  // Детектирование закрытых глаз
  if (detectClosedEyes(fb)) {
    eyesClosedCounter++;
    Serial.print("Счетчик закрытых глаз: ");
    Serial.println(eyesClosedCounter);
    
    if (eyesClosedCounter >= EYES_CLOSED_THRESHOLD) {
      // Отправка сигнала тревоги
      client.publish(topic, "ALARM");
      Serial.println("СИГНАЛ ТРЕВОГИ ОТПРАВЛЕН!");
      eyesClosedCounter = 0; // Сброс счетчика
    }
  } else {
    eyesClosedCounter = 0; // Сброс если глаза открыты
  }
  
  // Освобождение кадра
  esp_camera_fb_return(fb);
  
  delay(100); // Задержка между кадрами
}
