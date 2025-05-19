#include <ESP32Servo.h>   // ESP32‑compatible Servo library

Servo myServo;
const int servoPin    = 13;   // your wiring
const int stepAngle   = 90;   // how many degrees per step
const int moveDelayMs = 300;  // time your servo needs to move 90° (≈250 ms)
const int stopDelayMs = 2000; // 2 s stop time

int currentAngle = 0;         // tracks commanded position

void setup() {
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2400);
  myServo.write(currentAngle); // start at 0°
  delay(stopDelayMs);          // initial 2 s hold
  myServo.detach();            // power off hold torque
}

void loop() {
  // re‑enable the driver
  myServo.attach(servoPin, 500, 2400);

  // compute next absolute angle
  currentAngle += stepAngle;
  myServo.write(currentAngle);        // tell it to go +90° further

  delay(moveDelayMs);                 // **wait for it to finish the full 90°**

  myServo.detach();                   // then cut power so it “stops dead”
  delay(stopDelayMs);                 // hold that stopped position for 2 s
}
