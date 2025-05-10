#define trigPinRight 12
#define echoPinRight 13

#define trigPinLeft 6
#define echoPinLeft 7

// Motor control pins
#define IN1 11
#define IN2 10
#define IN3 9
#define IN4 8

// PID constants
float Kp = 2.0;
float Ki = 0.0;
float Kd = 1.2;

// PID variables
float error = 0, previous_error = 0;
float integral = 0, derivative = 0;
float correction = 0;

// Motor speed
int baseSpeed = 150;

// Setpoint: desired distance from right wall in cm
float setPoint = 15.0;

// Sound speed
#define SOUND_SPEED 0.034

void setup() {
  Serial.begin(9600);

  // Ultrasonic sensor pins
  pinMode(trigPinRight, OUTPUT);  pinMode(echoPinRight, INPUT);
  pinMode(trigPinLeft, OUTPUT);   pinMode(echoPinLeft, INPUT);

  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // Initial header
  Serial.println("=== Wall Following Robot ===");
  Serial.println("Right Wall Following using PID Control");
  Serial.println("======================================");
  delay(1000);
}

void loop() {
  float distanceRight = getDistance(trigPinRight, echoPinRight);
  float distanceLeft = getDistance(trigPinLeft, echoPinLeft);

  // PID calculation
  error = setPoint - distanceRight;
  integral += error;
  derivative = error - previous_error;
  correction = Kp * error + Ki * integral + Kd * derivative;
  previous_error = error;

  // Adjust speeds
  int leftSpeed = baseSpeed - correction;
  int rightSpeed = baseSpeed + correction;

  // Clamp speeds
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  // Serial output
  Serial.println("------ Robot Status ------");
  Serial.print("Left Distance  : "); Serial.print(distanceLeft); Serial.println(" cm");
  Serial.print("Right Distance : "); Serial.print(distanceRight); Serial.println(" cm");
  Serial.print("Error          : "); Serial.println(error);
  Serial.print("Correction     : "); Serial.println(correction);
  Serial.print("Left Motor PWM : "); Serial.println(leftSpeed);
  Serial.print("Right Motor PWM: "); Serial.println(rightSpeed);
  Serial.println("--------------------------\n");

  moveForward(leftSpeed, rightSpeed);

  delay(100);
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout at 30 ms
  float distance = duration * SOUND_SPEED / 2;
  return constrain(distance, 2, 200); // clamp for valid range
}

// Motor control
void moveForward(int leftSpeed, int rightSpeed) {
  analogWrite(IN1, leftSpeed); digitalWrite(IN2, LOW);
  analogWrite(IN3, rightSpeed); digitalWrite(IN4, LOW);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
