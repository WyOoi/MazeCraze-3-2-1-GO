#include <Wire.h>
#include <Adafruit_APDS9960.h>

// TCA9548A address
#define TCA_ADDR 0x70

// Create 3 APDS9960 sensor instances
Adafruit_APDS9960 apds[3];

// TCA9548A channel select
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize each APDS9960 on separate channels
  for (uint8_t i = 0; i < 3; i++) {
    tcaSelect(i);
    if (!apds[i].begin()) {
      Serial.print("APDS9960 #");
      Serial.print(i);
      Serial.println(" not found.");
    } else {
      apds[i].enableColor(true);
      Serial.print("APDS9960 #");
      Serial.print(i);
      Serial.println(" initialized.");
    }
    delay(100);
  }
}

void loop() {
  uint16_t r, g, b, c;

  for (uint8_t i = 0; i < 3; i++) {
    tcaSelect(i);
    if (apds[i].colorDataReady()) {
      apds[i].getColorData(&r, &g, &b, &c);
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(" - R: "); Serial.print(r);
      Serial.print(" G: "); Serial.print(g);
      Serial.print(" B: "); Serial.print(b);
      Serial.print(" C: "); Serial.println(c);
    }
    delay(1000);
  }

  Serial.println("---");
  delay(5000);
}
