// Pin definitions
const int trigPin1 = 5;   // Left sensor
const int echoPin1 = 18;
const int trigPin2 = 27;  // Right sensor
const int echoPin2 = 26;

// Motor control pins (example setup)
const int motorLeftPin = 32;
const int motorRightPin = 33;

// Sound speed
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// PID parameters
float Kp = 1.5;
float Ki = 0.05;
float Kd = 0.8;

float setPoint = 15.0; // Desired distance from wall in cm

float error = 0, previousError = 0, integral = 0, derivative = 0;
float correction = 0;

void setup() {
  Serial.begin(115200);

  // Ultrasonic sensor setup
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // Motor pins
  pinMode(motorLeftPin, OUTPUT);
  pinMode(motorRightPin, OUTPUT);
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * SOUND_SPEED / 2;
}

void loop() {
  float distanceLeft = getDistance(trigPin1, echoPin1);
  float distanceRight = getDistance(trigPin2, echoPin2);

  // Error: Positive if too far from right wall, negative if too close
  error = setPoint - distanceRight;

  // PID calculations
  integral += error;
  derivative = error - previousError;
  correction = Kp * error + Ki * integral + Kd * derivative;
  previousError = error;

  // Debugging output
  Serial.println("=== PID Wall Follower ===");
  Serial.print("Left Distance: "); Serial.println(distanceLeft);
  Serial.print("Right Distance: "); Serial.println(distanceRight);
  Serial.print("Error: "); Serial.println(error);
  Serial.print("Correction: "); Serial.println(correction);
  Serial.println("==========================");

  // Motor control (simple PWM logic)
  int baseSpeed = 150; // Adjust to your motors
  int leftSpeed = constrain(baseSpeed - correction, 0, 255);
  int rightSpeed = constrain(baseSpeed + correction, 0, 255);

  analogWrite(motorLeftPin, leftSpeed);
  analogWrite(motorRightPin, rightSpeed);

  delay(100);
}
