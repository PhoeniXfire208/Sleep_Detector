//NodeMCU V3
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Настройки Wi-Fi
const char* ssid = "PhoeniXfire";
const char* password = "34553455";

// Настройки MQTT
const char* mqtt_server = "test.mosquitto.org"; // Бесплатный публичный брокер
const char* topic = "driver/drowsiness";

// Пины
#define BUZZER_PIN 4  // D2
#define LED_PIN 5     // D1

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Подключение к ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi подключен");
  Serial.println("IP адрес: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Сообщение получено [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  if (message == "ALARM") {
    // Включаем сигнализацию
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("ТРЕВОГА! Водитель засыпает!");
    
    // Выключаем через 5 секунд
    delay(5000);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Подключение к MQTT брокеру...");
    if (client.connect("NodeMCUDriver")) {
      Serial.println("подключено");
      client.subscribe(topic);
    } else {
      Serial.print("ошибка, rc=");
      Serial.print(client.state());
      Serial.println(" пробуем через 5 секунд");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
