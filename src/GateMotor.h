#ifndef GATEMOTOR_H
#define GATEMOTOR_H

#include <Arduino.h>

void moveGate(bool closing);
void stopMotor(bool closing);
void softStart(bool closing);
void handleMotorStopAftermath();
void checkManualMovement();
void updateDisplay();

extern volatile int encoderPulseCount;
extern bool motorRunning;
extern bool gateOpen;
extern bool movementCompleted;
extern bool direction;

#endif
