#include <ESP32Servo.h>

#define JOY_X   34
#define JOY_Y   35
#define JOY_BTN 32
#define SERVO_PIN 26

Servo myServo;

void setup() {
  Serial.begin(115200);

  pinMode(JOY_BTN, INPUT_PULLUP);

  // ESP32 PWM için güvenli ayarlar
  myServo.setPeriodHertz(50);        // Servo standardı: 50Hz
  myServo.attach(SERVO_PIN, 500, 2400); 
  // 500–2400 µs → çoğu SG90 / MG90 uyumlu

  Serial.println("ESP32 Servo + Joystick basladi");
}

void loop() {
  int joyX = analogRead(JOY_X);   // 0–4095
  int joyY = analogRead(JOY_Y);   // Şimdilik sadece okunuyor
  int btn  = digitalRead(JOY_BTN);

  // X eksenini servo açısına çevir (0–180)
  int angle = map(joyX, 0, 4095, 0, 150);
  angle = constrain(angle, 0, 150);

  if (btn == LOW) {
    // Butona basılıysa servo ortala
    myServo.write(66);
    Serial.println("Buton basili → Servo 90°");
  } else {
    myServo.write(angle);
    Serial.print("Servo Acisi: ");
    Serial.println(angle);
  }

  delay(15); // Servo için ideal gecikme
}
