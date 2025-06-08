#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>

// Piny mostka H
constexpr int R_EN = 18;
constexpr int L_EN = 19;
constexpr int R_PWM = 21;
constexpr int L_PWM = 22;

// Piny enkodera
constexpr int encoderPinA = 32;
constexpr int encoderPinB = 33;

// OLED
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_SDA = 13;
constexpr int OLED_SCL = 14;

// WiFi i SinricPro
constexpr char WIFI_SSID[] = "T-Mobile_Swiatlowod_2.4GHz_8D01";
constexpr char WIFI_PASS[] = "ab6rrus77r7xcsck";
constexpr char APP_KEY[] = "859e7ba9-afbf-4881-9956-a668168bb197";
constexpr char APP_SECRET[] = "d6f280dd-f1d4-481f-8230-75207a82df60-1e86b2d2-6454-4037-9ac0-726be5ac782f";
constexpr char SWITCH_ID[] = "682481e9bddfc53e33f50cb1";

//pilot
#define RF_RECEIVER_PIN 27

//inne
constexpr int pulseLimit = 1620;
constexpr int manualThreshold = 1500;
constexpr int pwmValue = 160;
constexpr int minPWM = 55;
constexpr unsigned long ENCODER_TIMEOUT = 140;
constexpr unsigned long WiFiCheckInterval = 5000;
constexpr unsigned long SinricProReconnectInterval = 5000;

extern bool manualMovementEnabled;
extern unsigned long lastEncoderChangeTime;
extern int lastEncoderValue;
extern int adjustedPulseLimit;
extern unsigned long motorStopTime;
extern Preferences preferences;

extern volatile int encoderPulseCount;
extern bool motorRunning;
extern bool gateOpen;
extern bool movementCompleted;
extern bool direction;
extern bool sinricProConnected;
extern int manualMovementPulses;
extern int lastPulseCount;
extern bool sinricProConnected;
extern bool manualMovementEnabled;
extern unsigned long lastEncoderChangeTime;
extern int lastEncoderValue;
extern int adjustedPulseLimit;
extern unsigned long motorStopTime;

extern Adafruit_SSD1306 display;
#endif
