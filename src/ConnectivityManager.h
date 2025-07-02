#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <Arduino.h>
#include <RCSwitch.h>

bool connectToWiFi();
bool connectToSinricPro();
void handleConnectivity();
bool handleSinricGateRequest(const String &deviceId, bool &desiredState);
void handleRemoteControl();
void sendGateStateToSinricPro(bool isGateOpen);
void setupSinricProCallbacks();
void WiFiQuality();

extern bool sinricProConnected;
extern RCSwitch myRemoteSwitch;
extern int WiFiSignalQuality;

#endif
