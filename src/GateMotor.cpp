#include "GateMotor.h"
#include "Config.h"
#include <Preferences.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "ConnectivityManager.h"

extern volatile int encoderPulseCount;
extern bool motorRunning;
extern bool gateOpen;
extern bool movementCompleted;
extern bool direction;
extern int manualMovementPulses;
extern int lastPulseCount;

extern Preferences preferences;
extern Adafruit_SSD1306 display;
extern portMUX_TYPE mux;


void stopMotor(bool closing) {
  int currentPWM = (closing) ? analogRead(L_PWM) : analogRead(R_PWM);
  const int steps = 10;
  const int stepDelay = 30;
  
  for (int i = 0; i < steps; i++) {
    currentPWM -= currentPWM / (steps - i);
    currentPWM = max(currentPWM, 0);
    
    if (closing) {
      analogWrite(L_PWM, currentPWM);
    } else {
      analogWrite(R_PWM, currentPWM);
    }
    unsigned long start = millis();
    while (millis() - start < stepDelay) {
      yield(); 
    }
  }
  
  digitalWrite(R_EN, LOW);
  digitalWrite(L_EN, LOW);
  analogWrite(R_PWM, 0);
  analogWrite(L_PWM, 0);

  motorRunning = false;
  movementCompleted = true;
  gateOpen = !closing;
}

void softStart(bool closing) {
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  analogWrite(R_PWM, 0);
  analogWrite(L_PWM, 0);
  delay(5);

  motorRunning = true;
  direction = closing;

for (int pwm = 0; pwm <= pwmValue; pwm += 1) {
  if (closing) {
    analogWrite(R_PWM, 0);
    analogWrite(L_PWM, pwm);
  } else {
    analogWrite(R_PWM, pwm);
    analogWrite(L_PWM, 0);
  }
  unsigned long start = millis();
  while (millis() - start < 15) {
    yield();
  }
}
}

void moveGate(bool closing) {
  manualMovementEnabled = false;

  if (movementCompleted) {
    portENTER_CRITICAL(&mux);
    encoderPulseCount = 0;
    portEXIT_CRITICAL(&mux);
    lastEncoderValue = 0;
    lastEncoderChangeTime = millis();
  }

  adjustedPulseLimit = pulseLimit;

  preferences.begin("motor", false);
  int manualOffset = preferences.getInt("manualPulses", 0);
  preferences.putInt("manualPulses", 0);
  preferences.end();

  adjustedPulseLimit = pulseLimit - abs(manualOffset);
  
  bool brakingPhase = (adjustedPulseLimit < 180); 

  softStart(closing);

  const int brakingStart = adjustedPulseLimit * 0.82;
  bool movementStopped = false;

  while (!movementStopped) {
    int currentCount;
    portENTER_CRITICAL(&mux);
    currentCount = abs(encoderPulseCount);
    portEXIT_CRITICAL(&mux);

    if (currentCount != lastEncoderValue) {
      lastEncoderValue = currentCount;
      lastEncoderChangeTime = millis();
    } 
else if (motorRunning && (millis() - lastEncoderChangeTime > ENCODER_TIMEOUT)) {
  stopMotor(closing);
  updateDisplay();
  movementStopped = true;
}

if (brakingPhase || currentCount >= brakingStart) {
  int remainingDistance = adjustedPulseLimit - currentCount;

  int dynamicPWM = (remainingDistance > 180) 
      ? map(remainingDistance, 180, adjustedPulseLimit - brakingStart, minPWM, pwmValue) 
      : minPWM;

  dynamicPWM = constrain(dynamicPWM, minPWM, pwmValue);

  analogWrite(closing ? L_PWM : R_PWM, dynamicPWM);
}

    updateDisplay();

    if (currentCount >= adjustedPulseLimit) {
      stopMotor(closing);
      movementStopped = true;
    }
    updateDisplay();
    delay(10);
  }
  updateDisplay();
}

void handleMotorStopAftermath() {
  if (motorRunning || millis() - motorStopTime < 5000) return;

  if (!manualMovementEnabled && encoderPulseCount != 0) {
    noInterrupts();
    encoderPulseCount = 0;
    interrupts();
    manualMovementEnabled = true;
    manualMovementPulses = 0;
    lastPulseCount = 0;
    updateDisplay();
  }
}

void checkManualMovement() {
  if (!manualMovementEnabled) return;

  int currentPulseCount;
  portENTER_CRITICAL(&mux);
  currentPulseCount = encoderPulseCount;
  portEXIT_CRITICAL(&mux);

  if (currentPulseCount != lastPulseCount) {
    int delta = currentPulseCount - lastPulseCount;
    manualMovementPulses += delta;
    lastPulseCount = currentPulseCount;
    
    if (!motorRunning) {
      preferences.begin("motor", false);
      preferences.putInt("manualPulses", manualMovementPulses);
      preferences.end();
    }
    
    if (abs(manualMovementPulses) >= manualThreshold) {
      gateOpen = !gateOpen;
      manualMovementPulses = 0;
      
      preferences.begin("motor", false);
      preferences.putInt("manualPulses", 0);
      preferences.putBool("direction", !gateOpen);
      preferences.end();

      sendGateStateToSinricPro(gateOpen);
    }
    
    updateDisplay();
  }
}

