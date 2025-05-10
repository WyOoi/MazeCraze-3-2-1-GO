// Updated ESP32 sketch: separate floor and treasure color scanning functions
#include <Wire.h>
#include <Adafruit_APDS9960.h>

// TCA9548A I2C multiplexer address
#define TCA_ADDR 0x70

// Create 5 APDS9960 sensor instances
Adafruit_APDS9960 apds[5];

// Multiplexer channel select helper
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// --- COLOR THRESHOLD DEFINITIONS ---
// Floor colors based on HTML RGB values:
// Black  rgb(0,0,0)
// Red    rgb(255,0,0)
// Green  rgb(0,255,0)
// Yellow rgb(255,255,0)
// Blue   rgb(0,0,255)
const uint16_t TH_BLACK_F = 100;
const uint16_t TH_RED_F   = 200;
const uint16_t TH_GREEN_F = 200;
const uint16_t TH_BLUE_F  = 200;

// Treasure colors based on HTML RGB values:
// Cyan   rgb(0,255,255)
// Green  rgb(0,255,0)
// Black  rgb(0,0,0)
const uint16_t TH_CYAN_T   = 200;
const uint16_t TH_TGREEN_T = 200;
const uint16_t TH_BLACK_T  = 100;

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
      String col = getFloorColor(r, g, b);
      Serial.print("Floor Sensor #"); Serial.print(ch);
      Serial.print(": "); Serial.println(col);
    }
    delay(100);
  }
  Serial.println("--- Floor Scan Complete ---");
}

// 2) Scan treasures with 3 sensors (channels 2-4)
void scanTreasureColor() {
  uint16_t r, g, b, c;
  for (uint8_t ch = 2; ch < 5; ch++) {
    tcaSelect(ch);
    if (apds[ch].colorDataReady()) {
      apds[ch].getColorData(&r, &g, &b, &c);
      String col = getTreasureColor(r, g, b);
      Serial.print("Treasure Sensor #"); Serial.print(ch);
      Serial.print(": "); Serial.println(col);
    }
    delay(100);
  }
  Serial.println("--- Treasure Scan Complete ---");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  // Initialize all 5 APDS9960 sensors
  for (uint8_t i = 0; i < 5; i++) {
    tcaSelect(i);
    if (!apds[i].begin()) {
      Serial.print("APDS9960 #"); Serial.print(i);
      Serial.println(" initialization failed");
    } else {
      apds[i].enableColor(true);
      Serial.print("APDS9960 #"); Serial.print(i);
      Serial.println(" ready");
    }
    delay(100);
  }
}

void loop() {
  scanFloorColor();     // channels 0-1
  scanTreasureColor();  // channels 2-4
  delay(1000);
}
