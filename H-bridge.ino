// Define motor driver pins

// Motor A (Left Motor)
const int IN1 = 9;   // PWM pin connected to IN1
const int IN2 = 10;  // PWM pin connected to IN2

// Motor B (Right Motor)
const int IN3 = 5;   // PWM pin connected to IN3
const int IN4 = 6;   // PWM pin connected to IN4

void setup() {
  // Start serial communication
  Serial.begin(9600);

  // Set all motor pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.println("Setup complete. Starting motor control...");
}

void loop() {
  // Move both motors forward
  Serial.println("Moving forward...");
  analogWrite(IN1, 255); // Motor A forward
  analogWrite(IN2, 0);
  analogWrite(IN3, 255); // Motor B forward
  analogWrite(IN4, 0);
  delay(2000);

  // Stop both motors
  Serial.println("Stopping...");
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, 0);
  delay(1000);

  // Move both motors backward
  Serial.println("Reversing...");
  analogWrite(IN1, 0);
  analogWrite(IN2, 255); // Motor A reverse
  analogWrite(IN3, 0);
  analogWrite(IN4, 255); // Motor B reverse
  delay(2000);

  // Stop both motors
  Serial.println("Stopping...");
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, 0);
  delay(1000);
}
