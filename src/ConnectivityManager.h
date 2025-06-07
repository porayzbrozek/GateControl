#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <Arduino.h>

bool connectToWiFi();
bool connectToSinricPro();
void handleConnectivity();
bool onPowerState(const String &deviceId, bool &state);

extern bool sinricProConnected;

#endif
