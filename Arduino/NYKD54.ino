#include <ESP32Servo.h>   // ESP32‑compatible Servo library
Servo myServo;

void setup() {
  const int servoPin = 13;        // change to your wiring pin
  myServo.setPeriodHertz(50);     // SG90 expects a 50 Hz update rate
  // attach(pin, minPulse, maxPulse) in microseconds for 0°–180°
  myServo.attach(servoPin, 500, 2500);  // 500 µs → 0°, 2500 µs → 180° :contentReference[oaicite:0]{index=0}
}

void controlServoMax() {
  myServo.write(180);  // command full sweep to 180° :contentReference[oaicite:1]{index=1}
  delay(2000);         // hold position for 2 s
  myServo.write(0);    // return to 0°
}

void loop() {
  controlServoMax();   // use the helper each cycle (SG90 experiment on ESP32) :contentReference[oaicite:2]{index=2}
  delay(1000);         // optional 1 s pause before repeating
}
