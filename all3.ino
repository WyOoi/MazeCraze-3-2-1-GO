// ESP32 Sketch: Color Scanning, Ultrasonic DFS, Motor Control with Encoders
// - 5 APDS9960 color sensors (TCA9548A multiplexer)
// - 3 HC-SR04 ultrasonic sensors (wall detection using DFS)
// - Dual motors via L298N driver, with rotary encoder feedback

#include <Wire.h>
#include <Adafruit_APDS9960.h>

// --- TCA9548A I2C multiplexer ---
#define TCA_ADDR 0x70
Adafruit_APDS9960 apds[5];

// --- HC-SR04 ultrasonic definitions ---
const int trigPins[3] = {12, 14, 27};
const int echoPins[3] = {13, 25, 26};
const int adjacency[3][2] = {{1, -1}, {0, 2}, {1, -1}};

// --- L298N Motor driver pins ---
const int ENA = 25;
const int IN1 = 33;
const int IN2 = 32;
const int ENB = 14;
const int IN3 = 27;
const int IN4 = 26;

// --- Encoder pins and state ---
const int encA = 34;  // Left motor encoder A
const int encB = 35;  // Right motor encoder B
volatile long countA = 0;
volatile long countB = 0;

// --- Color thresholds ---
const uint16_t TH_BLACK_F = 100, TH_RED_F = 200, TH_GREEN_F = 200, TH_BLUE_F = 200;
const uint16_t TH_CYAN_T = 200, TH_TGREEN_T = 200, TH_BLACK_T = 100;

// --- Motor PWM configuration ---
const int frequency = 500;
const int motorLeft = 0;
const int motorRight = 1;
const int resolution = 8;

// --- Setup multiplexer channel select ---
void tcaSelect(uint8_t ch) {
  if (ch > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << ch);
  Wire.endTransmission();
}

// --- Color mapping ---
String getFloorColor(uint16_t r,uint16_t g,uint16_t b){
  if(r<TH_BLACK_F&&g<TH_BLACK_F&&b<TH_BLACK_F)return"Black";
  if(r>TH_RED_F&&g<TH_BLACK_F&&b<TH_BLACK_F)return"Red";
  if(r<TH_BLACK_F&&g>TH_GREEN_F&&b<TH_BLACK_F)return"Green";
  if(r>TH_RED_F&&g>TH_GREEN_F&&b<TH_BLACK_F)return"Yellow";
  if(r<TH_BLACK_F&&g<TH_BLACK_F&&b>TH_BLUE_F)return"Blue";
  return"Unknown";
}
String getTreasureColor(uint16_t r,uint16_t g,uint16_t b){
  if(g>TH_CYAN_T&&b>TH_CYAN_T&&r<TH_BLACK_T)return"Cyan";
  if(g>TH_TGREEN_T&&r<TH_BLACK_T&&b<TH_BLACK_T)return"Green";
  if(r<TH_BLACK_T&&g<TH_BLACK_T&&b<TH_BLACK_T)return"Black";
  return"Invalid";
}

// --- Scanning functions ---
void scanFloorColor(){uint16_t r,g,b,c;for(uint8_t ch=0;ch<2;ch++){tcaSelect(ch);if(apds[ch].colorDataReady()){apds[ch].getColorData(&r,&g,&b,&c);Serial.printf("Floor#%u: %s\n",ch,getFloorColor(r,g,b).c_str());}delay(100);}Serial.println("---Floor Scan Done---");}
void scanTreasureColor(){uint16_t r,g,b,c;for(uint8_t ch=2;ch<5;ch++){tcaSelect(ch);if(apds[ch].colorDataReady()){apds[ch].getColorData(&r,&g,&b,&c);Serial.printf("Treasure#%u: %s\n",ch,getTreasureColor(r,g,b).c_str());}delay(100);}Serial.println("---Treasure Scan Done---");}
long readDistance(int i){digitalWrite(trigPins[i],LOW);delayMicroseconds(2);digitalWrite(trigPins[i],HIGH);delayMicroseconds(10);digitalWrite(trigPins[i],LOW);long dur=pulseIn(echoPins[i],HIGH,30000);return dur*0.034/2;}
void scanUltrasonicDFS(int idx,bool vis[]){if(idx<0||idx>=3||vis[idx])return;vis[idx]=true;Serial.printf("US#%d: %ldcm\n",idx,readDistance(idx));for(int i=0;i<2;i++)scanUltrasonicDFS(adjacency[idx][i],vis);} 
void scanWallDFS(){bool vis[3]={false,false,false};scanUltrasonicDFS(0,vis);Serial.println("---US DFS Done---");}

// --- Encoder ISRs ---
void IRAM_ATTR onEncA(){countA++;}
void IRAM_ATTR onEncB(){countB++;}

// --- Motor control ---
void initMotor(){pinMode(ENA,OUTPUT);pinMode(IN1,OUTPUT);pinMode(IN2,OUTPUT);
  pinMode(ENB,OUTPUT);pinMode(IN3,OUTPUT);pinMode(IN4,OUTPUT);
  ledcSetup(motorLeft,frequency,resolution);ledcAttachPin(ENA,motorLeft);
  ledcSetup(motorRight,frequency,resolution);ledcAttachPin(ENB,motorRight);
}
void stopMotors(){digitalWrite(IN1,LOW);digitalWrite(IN2,LOW);ledcWrite(motorLeft,0);
                  digitalWrite(IN3,LOW);digitalWrite(IN4,LOW);ledcWrite(motorRight,0);}
void forward(int spdL,int spdR){digitalWrite(IN1,LOW);digitalWrite(IN2,HIGH);ledcWrite(motorLeft,spdL);
                             digitalWrite(IN3,HIGH);digitalWrite(IN4,LOW);ledcWrite(motorRight,spdR);}    
void reverse(int spd){digitalWrite(IN1,HIGH);digitalWrite(IN2,LOW);ledcWrite(motorLeft,spd);
                       digitalWrite(IN3,LOW);digitalWrite(IN4,HIGH);ledcWrite(motorRight,spd);}
void turnLeft(int spd){digitalWrite(IN1,HIGH);digitalWrite(IN2,LOW);ledcWrite(motorLeft,spd);
                        digitalWrite(IN3,HIGH);digitalWrite(IN4,LOW);ledcWrite(motorRight,0);}  
void turnRight(int spd){digitalWrite(IN1,LOW);digitalWrite(IN2,HIGH);ledcWrite(motorLeft,0);
                         digitalWrite(IN3,LOW);digitalWrite(IN4,HIGH);ledcWrite(motorRight,spd);}  

void setup(){
  Serial.begin(115200);Wire.begin();
  // Color sensors init
  for(uint8_t i=0;i<5;i++){tcaSelect(i);if(!apds[i].begin())Serial.printf("APDS#%u fail\n",i);else{apds[i].enableColor(true);Serial.printf("APDS#%u ready\n",i);}delay(100);}  
  // Ultrasonic pins
  for(int i=0;i<3;i++){pinMode(trigPins[i],OUTPUT);pinMode(echoPins[i],INPUT);}  
  // Motor + encoder
  initMotor();
  pinMode(encA,INPUT_PULLUP);attachInterrupt(digitalPinToInterrupt(encA),onEncA,RISING);
  pinMode(encB,INPUT_PULLUP);attachInterrupt(digitalPinToInterrupt(encB),onEncB,RISING);
}

void loop(){
  scanFloorColor();scanTreasureColor();scanWallDFS();
  // Example drive: move forward until encoder A reaches 100 ticks
  if(countA<100){forward(150,150);}else{stopMotors();}
  delay(1000);
}
