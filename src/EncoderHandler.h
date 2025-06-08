#ifndef ENCODERHANDLER_H
#define ENCODERHANDLER_H

#include <Arduino.h>

void IRAM_ATTR updateEncoder();

extern volatile int encoderPulseCount;
extern portMUX_TYPE mux;

#endif
