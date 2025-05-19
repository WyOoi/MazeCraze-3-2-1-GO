// --- ULTRASONIC PINS ---
const uint8_t TRIG_LEFT   = 12, ECHO_LEFT   = 34;
const uint8_t TRIG_CENTER = 27, ECHO_CENTER = 36;
const uint8_t TRIG_RIGHT  = 14, ECHO_RIGHT  = 39;

// --- ZK-BM1 MOTOR DRIVER PINS ---
const uint8_t IN1 = 25;
const uint8_t IN2 = 26;
const uint8_t IN3 = 33;
const uint8_t IN4 = 32;

// --- PWM CHANNELS & SETTINGS ---
const uint8_t  CH_A1    = 0;
const uint8_t  CH_A2    = 1;
const uint8_t  CH_B1    = 2;
const uint8_t  CH_B2    = 3;
const uint32_t PWM_FREQ = 500;
const uint8_t  PWM_RES  = 8;

// --- SPEED & TIMING ---
const int speedFwd     = 150;
const int speedRev     = 140;
const int speedTurn    = 170;
const long OBSTACLE_CM = 15;   // threshold for center sensor
const int  backMs      = 1000;  // ms to reverse
const int  turnMs      = 3000;  // ms to pivot 90°

void setMotors(int16_t sL, int16_t sR) {
  // Motor A
  if (sL >= 0) { ledcWrite(CH_A1, sL); ledcWrite(CH_A2, 0); }
  else         { ledcWrite(CH_A1, 0);   ledcWrite(CH_A2, -sL); }
  // Motor B
  if (sR >= 0) { ledcWrite(CH_B1, sR); ledcWrite(CH_B2, 0); }
  else         { ledcWrite(CH_B1, 0);   ledcWrite(CH_B2, -sR); }
}
void forward()  { setMotors( speedFwd,  speedFwd); }
void reverse()  { setMotors(-speedRev, -speedRev); }
void turnLeft() { setMotors(-speedTurn, speedTurn); delay(turnMs); setMotors(0,0); }
void turnRight(){ setMotors( speedTurn,-speedTurn); delay(turnMs); setMotors(0,0); }
void stopBot()  { setMotors(0,0); }

// Read HC-SR04 (cm; –1 on timeout)
long readUltra(uint8_t trig, uint8_t echo) {
  digitalWrite(trig, LOW);  delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long us = pulseIn(echo, HIGH, 30000);
  return us ? (us * 0.034 / 2) : -1;
}

void initMotor() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  ledcSetup(CH_A1, PWM_FREQ, PWM_RES); ledcAttachPin(IN1, CH_A1);
  ledcSetup(CH_A2, PWM_FREQ, PWM_RES); ledcAttachPin(IN2, CH_A2);
  ledcSetup(CH_B1, PWM_FREQ, PWM_RES); ledcAttachPin(IN3, CH_B1);
  ledcSetup(CH_B2, PWM_FREQ, PWM_RES); ledcAttachPin(IN4, CH_B2);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_LEFT,   OUTPUT); pinMode(ECHO_LEFT,   INPUT);
  pinMode(TRIG_CENTER, OUTPUT); pinMode(ECHO_CENTER, INPUT);
  pinMode(TRIG_RIGHT,  OUTPUT); pinMode(ECHO_RIGHT,  INPUT);
  initMotor();
}

enum State {
  MOVE_FORWARD,
  BACK_UP,
  READ_SIDES,
  TURN,
};

State currentState = MOVE_FORWARD;
long dL, dC, dR;

void loop() {
  switch (currentState) {

    case MOVE_FORWARD:
      // keep driving until obstacle
      dC = readUltra(TRIG_CENTER, ECHO_CENTER);
      if (dC > 0 && dC < OBSTACLE_CM) {
        stopBot();
        Serial.println("Obstacle ahead!");
        currentState = BACK_UP;
      } else {
        forward();
        delay(50);
      }
      break;

    case BACK_UP:
      // reverse for a fixed time, then stop and go read sides
      reverse();
      delay(backMs);
      stopBot();
      delay(500);
      currentState = READ_SIDES;
      break;

    case READ_SIDES:
      // sample left & right
      dL = readUltra(TRIG_LEFT, ECHO_LEFT);
      dR = readUltra(TRIG_RIGHT, ECHO_RIGHT);
      Serial.printf("Left:%4ld  Right:%4ld\n", dL, dR);
      currentState = TURN;
      break;

    case TURN:
      // choose the larger opening
      if (dR > dL) {
        Serial.println("Turning RIGHT");
        turnRight();
        stopBot();
        delay(500);
      } else {
        Serial.println("Turning LEFT");
        turnLeft();
        delay(500);
      }
      // after turning, go back to driving forward
      currentState = MOVE_FORWARD;
      break;
  }

  // tiny debounce between state executions
  delay(50);
}

