// Updated ESP32 sketch: color scanning + new maze ultrasonic function
#include <Wire.h>
#include <Adafruit_APDS9960.h>

// TCA9548A I2C multiplexer address
#define TCA_ADDR 0x70
Adafruit_APDS9960 apds[5];

// Multiplexer channel select helper
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// --- COLOR THRESHOLD DEFINITIONS ---
const uint16_t TH_BLACK_F = 100, TH_RED_F = 200, TH_GREEN_F = 200, TH_BLUE_F = 200;
const uint16_t TH_CYAN_T = 200, TH_TGREEN_T = 200, TH_BLACK_T = 100;

// Map raw RGB to floor color
String getFloorColor(uint16_t r, uint16_t g, uint16_t b) {
  if (r < TH_BLACK_F && g < TH_BLACK_F && b < TH_BLACK_F) return "Black";
  if (r > TH_RED_F   && g < TH_BLACK_F && b < TH_BLACK_F) return "Red";
  if (r < TH_BLACK_F && g > TH_GREEN_F && b < TH_BLACK_F) return "Green";
  if (r > TH_RED_F   && g > TH_GREEN_F && b < TH_BLACK_F) return "Yellow";
  if (r < TH_BLACK_F && g < TH_BLACK_F && b > TH_BLUE_F ) return "Blue";
  return "Unknown";
}
// Map raw RGB to treasure color
String getTreasureColor(uint16_t r, uint16_t g, uint16_t b) {
  if (g > TH_CYAN_T   && b > TH_CYAN_T   && r < TH_BLACK_T) return "Cyan";
  if (g > TH_TGREEN_T && r < TH_BLACK_T && b < TH_BLACK_T) return "Green";
  if (r < TH_BLACK_T && g < TH_BLACK_T && b < TH_BLACK_T)  return "Black";
  return "Invalid";
}

// 1) Scan floor with 2 sensors (channels 0-1)
void scanFloorColor() {
  uint16_t r, g, b, c;
  for (uint8_t ch = 0; ch < 2; ch++) {
    tcaSelect(ch);
    if (apds[ch].colorDataReady()) {
      apds[ch].getColorData(&r, &g, &b, &c);
      Serial.printf("Floor[%u]: %s\n", ch, getFloorColor(r,g,b).c_str());
    }
    delay(100);
  }
  Serial.println("-- Floor Scan Complete --");
}

// 2) Scan treasures with 3 sensors (channels 2-4)
void scanTreasureColor() {
  uint16_t r, g, b, c;
  for (uint8_t ch = 2; ch < 5; ch++) {
    tcaSelect(ch);
    if (apds[ch].colorDataReady()) {
      apds[ch].getColorData(&r, &g, &b, &c);
      Serial.printf("Treasure[%u]: %s\n", ch, getTreasureColor(r,g,b).c_str());
    }
    delay(100);
  }
  Serial.println("-- Treasure Scan Complete --");
}

// --- MAZE ULTRASONIC SENSORS (LEFT, CENTER, RIGHT) ---
const uint8_t MAZE_TRIG_LEFT   = 14;
const uint8_t MAZE_ECHO_LEFT   = 27;
const uint8_t MAZE_TRIG_CENTER = 32;
const uint8_t MAZE_ECHO_CENTER = 35;
const uint8_t MAZE_TRIG_RIGHT  = 18;
const uint8_t MAZE_ECHO_RIGHT  = 19;

// Generic read for an HC-SR04 sensor
long readUltrasonic(uint8_t trigPin, uint8_t echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return (duration > 0) ? (duration * 0.034 / 2) : -1;
}

// New: scan maze sensors and print left/center/right distances
void scanMazeUltrasonics() {
  long left   = readUltrasonic(MAZE_TRIG_LEFT,   MAZE_ECHO_LEFT);
  long center = readUltrasonic(MAZE_TRIG_CENTER, MAZE_ECHO_CENTER);
  long right  = readUltrasonic(MAZE_TRIG_RIGHT,  MAZE_ECHO_RIGHT);
  Serial.printf("Maze US: L=%dcm C=%ldcm R=%ldcm\n", left, center, right);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize color sensors on TCA multiplexer
  for (uint8_t i = 0; i < 5; i++) {
    tcaSelect(i);
    apds[i].begin();
    apds[i].enableColor(true);
  }

  // Configure maze ultrasonic pins
  pinMode(MAZE_TRIG_LEFT,   OUTPUT);
  pinMode(MAZE_ECHO_LEFT,   INPUT);
  pinMode(MAZE_TRIG_CENTER, OUTPUT);
  pinMode(MAZE_ECHO_CENTER, INPUT);
  pinMode(MAZE_TRIG_RIGHT,  OUTPUT);
  pinMode(MAZE_ECHO_RIGHT,  INPUT);

  // Ensure trigger pins start LOW
  digitalWrite(MAZE_TRIG_LEFT,   LOW);
  digitalWrite(MAZE_TRIG_CENTER, LOW);
  digitalWrite(MAZE_TRIG_RIGHT,  LOW);
}

void loop() {
}
