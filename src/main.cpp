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

//stany
bool isGateOpen = false; // isGateOpen == true  → brama OTWARTA // isGateOpen == false → brama ZAMKNIĘTA
bool motorRunning = false;
bool movementCompleted = true;
bool manualMovementEnabled = false;

//liczniki
int manualMovementPulses = 0;
int lastPulseCount = 0;
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

  preferences.begin("gate", false); //false = read/write, true = read-only
  movementCompleted = preferences.getBool("moveComp", true); //("nazwa_zmiennej", wartosc_domyslna)
  isGateOpen = preferences.getBool("isGateOpen", false);
  encoderPulseCount = preferences.getInt("encPulseCnt", 0);
  manualMovementPulses = preferences.getInt("manMovePuls", 0);
  preferences.end();

  if (movementCompleted) {
    encoderPulseCount = 0;
    manualMovementPulses = 0;

    preferences.begin("gate", false);
    preferences.putInt("encPulseCnt", 0);
    preferences.putInt("manMovePuls", 0);
    preferences.putBool("moveComp", true);
    preferences.putBool("isGateOpen", isGateOpen);
    preferences.end();
  }

  //i2c
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Blad OLED!"));
    while (true);
  }

  display.clearDisplay();
  display.display(); //bufor -> wyświetlacz

  if (connectToWiFi()) {
    connectToSinricPro();
    sendGateStateToSinricPro(isGateOpen); 
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
  WiFiQuality();
  updateDisplay();
}
