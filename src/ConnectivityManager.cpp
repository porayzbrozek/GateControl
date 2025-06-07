#include "ConnectivityManager.h"
#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include "Config.h"
#include "GateMotor.h"
#include <Preferences.h>

bool sinricProConnected = false;
unsigned long lastWiFiCheck = 0;
unsigned long lastSinricProReconnectAttempt = 0;

bool connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;

  Serial.println("Laczenie z WiFi...");

  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nPolaczono z WiFi!");
    return true;
  }
}


bool connectToSinricPro() {
  if (WiFi.status() != WL_CONNECTED) return false;

  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
  mySwitch.onPowerState(onPowerState);
  
  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);


  unsigned long startTime = millis();
  while (!SinricPro.isConnected() && millis() - startTime < 5000) {
    SinricPro.handle();
    yield();
  }

  sinricProConnected = SinricPro.isConnected();
  if (sinricProConnected) {
    Serial.println("Polaczono z SinricPro!");
  } else {
    Serial.println("Rozlaczono z SinricPro!");
  }
  return sinricProConnected;
}


void handleConnectivity() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastWiFiCheck >= WiFiCheckInterval) {
    lastWiFiCheck = currentMillis;
    if (!connectToWiFi()) {
      sinricProConnected = false;
    } else if (!sinricProConnected) {
      connectToSinricPro();
    }
  }

  if (WiFi.status() == WL_CONNECTED && !sinricProConnected &&
      currentMillis - lastSinricProReconnectAttempt >= SinricProReconnectInterval) {
    lastSinricProReconnectAttempt = currentMillis;
    connectToSinricPro();
  }

  if (sinricProConnected) {
    SinricPro.handle();
  }
}

bool onPowerState(const String &deviceId, bool &state) {
  if (motorRunning) return false;
  
  if (movementCompleted && gateOpen == !state) {
    return true;
  }

  direction = state;
  moveGate(direction);

  preferences.begin("motor", false);
  preferences.putBool("direction", direction);
  preferences.end();

  return true;
}