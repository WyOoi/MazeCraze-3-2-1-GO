// Motor A (Left Motor)
const int IN1 = 16;   // PWM pin connected to IN1
const int IN2 = 17;   // PWM pin connected to IN2

// Motor B (Right Motor)
const int IN3 = 18;   // PWM pin connected to IN3
const int IN4 = 19;   // PWM pin connected to IN4

// PWM Configuration
const int freq = 1000;
const int resolution = 8;

// Channels for each pin (0–15 allowed on ESP32)
const int ch_IN1 = 0;
const int ch_IN2 = 1;
const int ch_IN3 = 2;
const int ch_IN4 = 3;

void setup() {
  Serial.begin(9600);

  // Set up PWM channels and attach to GPIOs
  ledcSetup(ch_IN1, freq, resolution);
  ledcAttachPin(IN1, ch_IN1);

  ledcSetup(ch_IN2, freq, resolution);
  ledcAttachPin(IN2, ch_IN2);

  ledcSetup(ch_IN3, freq, resolution);
  ledcAttachPin(IN3, ch_IN3);

  ledcSetup(ch_IN4, freq, resolution);
  ledcAttachPin(IN4, ch_IN4);

  Serial.println("Setup complete. Starting motor control...");
}

void loop() {
  // Move both motors forward
  Serial.println("Moving forward...");
  ledcWrite(ch_IN1, 255); // Motor A forward
  ledcWrite(ch_IN2, 0);
  ledcWrite(ch_IN3, 255); // Motor B forward
  ledcWrite(ch_IN4, 0);
  delay(2000);

  // Stop both motors
  Serial.println("Stopping...");
  ledcWrite(ch_IN1, 0);
  ledcWrite(ch_IN2, 0);
  ledcWrite(ch_IN3, 0);
  ledcWrite(ch_IN4, 0);
  delay(1000);

  // Move both motors backward
  Serial.println("Reversing...");
  ledcWrite(ch_IN1, 0);
  ledcWrite(ch_IN2, 255); // Motor A reverse
  ledcWrite(ch_IN3, 0);
  ledcWrite(ch_IN4, 255); // Motor B reverse
  delay(2000);

  // Stop both motors
  Serial.println("Stopping...");
  ledcWrite(ch_IN1, 0);
  ledcWrite(ch_IN2, 0);
  ledcWrite(ch_IN3, 0);
  ledcWrite(ch_IN4, 0);
  delay(1000);
}
