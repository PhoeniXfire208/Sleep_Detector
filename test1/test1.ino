void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);   // Включаем светодиод (он инверсный)
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);  // Выключаем светодиод
  delay(1000);
}
