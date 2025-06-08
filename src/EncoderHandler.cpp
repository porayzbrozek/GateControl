#include "EncoderHandler.h"
#include "Config.h"

volatile int encoderPulseCount = 0;
int lastEncoded = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR updateEncoder() {
  int MSB = digitalRead(encoderPinA);
  int LSB = digitalRead(encoderPinB);
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  portENTER_CRITICAL_ISR(&mux);
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderPulseCount++;
  else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderPulseCount--;
  portEXIT_CRITICAL_ISR(&mux);

  lastEncoded = encoded;
}
