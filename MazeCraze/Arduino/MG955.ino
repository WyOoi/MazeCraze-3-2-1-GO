#include <ESP32Servo.h>   // ESP32‑compatible Servo library
Servo myServo;

void setup() {
  const int servoPin = 13;             // Update this to your actual wiring
  myServo.setPeriodHertz(50);          // MG995 expects ~50 Hz signal
  myServo.attach(servoPin, 500, 2400); // MG995: 500 µs → 0°, 2400 µs → 180°

  myServo.write(90);    // Move to 90°
  delay(5000);          // Stay at 90° for 1 second

  myServo.write(180);   // Continue to 180°
}

void loop() {
  // No further action; stays at 180°
}
