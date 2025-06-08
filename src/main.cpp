#include <Arduino.h>
#include "GateMotor.h"
#include "EncoderHandler.h"
#include "DisplayManager.h"
#include "ConnectivityManager.h"
#include "Config.h"
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <RCSwitch.h>

Preferences preferences;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool motorRunning = false;
bool gateState = false;
bool movementCompleted = true;
bool direction = false;
int manualMovementPulses = 0;
int lastPulseCount = 0;
bool manualMovementEnabled = false;
unsigned long lastEncoderChangeTime = 0;
int lastEncoderValue = 0;
int adjustedPulseLimit = 0;
unsigned long motorStopTime = 0;


void setup() {
  Serial.begin(115200);

  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  pinMode(L_PWM, OUTPUT);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  preferences.begin("motor", false);
  direction = preferences.getBool("direction", false);
  encoderPulseCount = preferences.getInt("pulseCount", 0);
  movementCompleted = preferences.getBool("completed", true);
  manualMovementPulses = preferences.getInt("manualPulses", 0);
  preferences.end();

  if (movementCompleted) {
    gateState = preferences.getBool("gateState", false);
    encoderPulseCount = 0;
    manualMovementPulses = 0;

    preferences.begin("motor", false);
    preferences.putInt("pulseCount", 0);
    preferences.putInt("manualPulses", 0);
    preferences.end();
  } 
  else {
    gateState = preferences.getBool("gateState", false);
  }

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Blad OLED!"));
    while (true);
  }

  display.clearDisplay();
  display.display();

  if (connectToWiFi()) {
    connectToSinricPro();
  }

  setupSinricProCallbacks();

  myRemoteSwitch.enableReceive(digitalPinToInterrupt(RF_RECEIVER_PIN));
  updateDisplay();
}


void loop() {
  handleConnectivity();
  handleMotorStopAftermath();
  checkManualMovement();
  handleRemoteControl();
  updateDisplay();
}
