#include "ConnectivityManager.h"
#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include "Config.h"
#include "GateMotor.h"
#include <RCSwitch.h>

RCSwitch myRemoteSwitch = RCSwitch();

bool sinricProConnected = false;
unsigned long lastWiFiCheck = 0;
unsigned long lastSinricProReconnectAttempt = 0;


bool connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;

  Serial.println("Laczenie z WiFi...");

  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
for(int i = 0; i<=20; i++){
if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nPolaczono z WiFi!");
    return true;
  }
    delay(500);
    Serial.print(".");  
}
    Serial.println("\nBłąd połączenia!");
    return false;
    
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
  
  if (movementCompleted && gateState != state) {
    return true;
  }

  direction = state;
  moveGate(direction);

  preferences.begin("motor", false);
  preferences.putBool("direction", direction);
  preferences.end();

  return true;
}


void handleRemoteControl() {
  if (myRemoteSwitch.available()) {
    unsigned long receivedCode = myRemoteSwitch.getReceivedValue();

    if (receivedCode == REMOTE_TOGGLE_CODE) {
      if (!motorRunning && movementCompleted) {
        direction = !gateState; 
          moveGate(direction);

          preferences.begin("motor", false);
          preferences.putBool("direction", direction);
          preferences.end();

          SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
          mySwitch.sendPowerStateEvent(direction);
        }
      }
    }

    myRemoteSwitch.resetAvailable();
  }




void sinricProOnConnected() {
  sinricProConnected = true;
  Serial.println("SinricPro connected!");
  updateDisplay(); 
}


void sinricProOnDisconnected() {
  sinricProConnected = false;
  Serial.println("SinricPro disconnected!");
  updateDisplay(); 
}


void setupSinricProCallbacks() {
  SinricPro.onConnected(sinricProOnConnected);
  SinricPro.onDisconnected(sinricProOnDisconnected);
}


void sendGateStateToSinricPro(bool gateState) {
  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
  mySwitch.sendPowerStateEvent(!gateState); // Wyślij do SinricPro aktualny stan bramy (true=zamknięta)

}