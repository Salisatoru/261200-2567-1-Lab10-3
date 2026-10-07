#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;

// ขา DC Motor หลักชุดที่ 1
const int MOTOR1_IN1 = 16;
const int MOTOR1_IN2 = 17;
const int MOTOR2_IN3 = 27;
const int MOTOR2_IN4 = 26;

// ขาควบคุม Relay สำหรับ Motor 3 (Active HIGH)
const int MOTOR3_RELAY = 13; 

// กำหนด Servo Motor 3 ตัว
Servo servo1;
Servo servo2;
Servo servo3;

const int SERVO1_PIN = 19;
const int SERVO2_PIN = 32;
const int SERVO3_PIN = 33;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayMotorState(String text) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.print(text);
  display.display();
}

// 1. เดินหน้า (แก้ไขสลับพินให้ล้อหมุนไปในทิศทางเดียวกัน)
void moveForward() {
  digitalWrite(MOTOR1_IN1, LOW);   // ปรับสลับสถานะเพื่อให้ล้อฝั่งที่กลับด้านหมุนถูกต้อง
  digitalWrite(MOTOR1_IN2, HIGH);  
  digitalWrite(MOTOR2_IN3, LOW);  
  digitalWrite(MOTOR2_IN4, HIGH); 
  displayMotorState("Forward");
}

// 2. ถอยหลัง (สลับตรงข้ามกับเดินหน้า)
void moveBackward() {
  digitalWrite(MOTOR1_IN1, HIGH); 
  digitalWrite(MOTOR1_IN2, LOW);  
  digitalWrite(MOTOR2_IN3, HIGH); 
  digitalWrite(MOTOR2_IN4, LOW);
  displayMotorState("Backward");
}

// 3. เลี้ยวซ้าย
void turnLeft() {
  digitalWrite(MOTOR1_IN1, HIGH);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN3, HIGH);
  digitalWrite(MOTOR2_IN4, LOW);
  displayMotorState("Turn Left");
}

// 4. เลี้ยวขวา
void turnRight() {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, HIGH);
  digitalWrite(MOTOR2_IN3, LOW);
  digitalWrite(MOTOR2_IN4, HIGH);
  displayMotorState("Turn Right");
}

void motor3Action() {
  digitalWrite(MOTOR3_RELAY, HIGH); 
  displayMotorState("Motor 3 ON");
}

void stopMotors() {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN3, LOW);
  digitalWrite(MOTOR2_IN4, LOW);
  digitalWrite(MOTOR3_RELAY, LOW); 
}

void processCommand(String input) {
  input.trim();
  input.toLowerCase(); 
  if (input.length() == 0) return;

  if (input == "f" || input == "forward") {
    moveForward();  
  }
  else if (input == "b" || input == "backward") {
    moveBackward(); 
  }
  else if (input == "l" || input == "left") {
    turnLeft();     
  }
  else if (input == "r" || input == "right") {
    turnRight();    
  }
  else if (input == "3" || input == "motor3") {
    motor3Action();
  }
  // ควบคุม Servo 1
  else if (input == "s1" || input == "servo1") {
    servo1.write(180);
    displayMotorState("Servo 1: 180");
  }
  else if (input == "s1_0") {
    servo1.write(0);
    displayMotorState("Servo 1: 0");
  }
  // ควบคุม Servo 2
  else if (input == "s2" || input == "servo2") {
    servo2.write(180);
    displayMotorState("Servo 2: 180");
  }
  else if (input == "s2_0") {
    servo2.write(0);
    displayMotorState("Servo 2: 0");
  }
  // ควบคุม Servo 3
  else if (input == "s3" || input == "servo3") {
    servo3.write(180);
    displayMotorState("Servo 3: 180");
  }
  else if (input == "s3_0") {
    servo3.write(0);
    displayMotorState("Servo 3: 0");
  }
  // คำสั่งหยุดมอเตอร์ (ทำงานเมื่อปล่อยปุ่มบนรีโมท BLE)
  else if (input == "0" || input == "stop" || input == "s") {
    stopMotors();
    displayMotorState("Stopped");
  }
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      displayMotorState("Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      displayMotorState("BLE Ready");
      pServer->getAdvertising()->start();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String input = pCharacteristic->getValue();
      if (input.length() > 0) {
        processCommand(input);
      }
    }
};

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  displayMotorState("BLE Ready");

  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN3, OUTPUT);
  pinMode(MOTOR2_IN4, OUTPUT);
  pinMode(MOTOR3_RELAY, OUTPUT);
  
  ESP32PWM::allocate