#include "GateMotor.h"
#include "Config.h"
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "ConnectivityManager.h"

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
  isGateOpen = !closing;

  preferences.begin("gate", false);
  preferences.putBool("isGateOpen", isGateOpen);
  preferences.putBool("movementCompleted", true);
  preferences.end();
}

void softStart(bool closing) {
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  analogWrite(R_PWM, 0);
  analogWrite(L_PWM, 0);
  delay(5);

  motorRunning = true;

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

  preferences.begin("gate", false);
  int manualOffset = preferences.getInt("manualMovementPulses", 0);
  preferences.putInt("manualMovementPulses", 0);
  preferences.end();

  adjustedPulseLimit = pulseLimit - abs(manualOffset);

  bool brakingPhase = adjustedPulseLimit < 180;
  const int brakingStart = adjustedPulseLimit * 0.82;
  bool movementStopped = false;

  softStart(closing);

  while (!movementStopped) {
    int currentCount;
    portENTER_CRITICAL(&mux);
    currentCount = abs(encoderPulseCount);
    portEXIT_CRITICAL(&mux);

    if (currentCount != lastEncoderValue) {
      lastEncoderValue = currentCount;
      lastEncoderChangeTime = millis();
    }

    if (motorRunning && (millis() - lastEncoderChangeTime > ENCODER_TIMEOUT)) {
      stopMotor(closing);
      updateDisplay();
      movementStopped = true;
      break;
    }

    if (!brakingPhase && currentCount >= brakingStart) {
      brakingPhase = true;
    }

    if (brakingPhase) {
      int remainingDistance = adjustedPulseLimit - currentCount;
      int dynamicPWM = (remainingDistance > 180)
        ? map(remainingDistance, 180, adjustedPulseLimit - brakingStart, minPWM, pwmValue)
        : minPWM;

      dynamicPWM = constrain(dynamicPWM, minPWM, pwmValue);
      analogWrite(closing ? L_PWM : R_PWM, dynamicPWM);
    }

    updateDisplay();
    delay(10);
  }

  updateDisplay();
}

void handleMotorStopAftermath() {
  if (motorRunning || millis() - motorStopTime < 1000) return;

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

    static unsigned long lastSaveTime = 0;

    if (!motorRunning && millis() - lastSaveTime > 3000) {
      preferences.begin("gate", false);
      preferences.putInt("manualMovementPulses", manualMovementPulses);
      preferences.end();
      lastSaveTime = millis();
    }

    if (abs(manualMovementPulses) >= manualThreshold) {
      isGateOpen = !isGateOpen;
      manualMovementPulses = 0;

      preferences.begin("gate", false);
      preferences.putInt("manualMovementPulses", 0);
      preferences.putBool("isGateOpen", isGateOpen);
      preferences.end();

      sendGateStateToSinricPro(isGateOpen);
    }

    updateDisplay();
  }
}
