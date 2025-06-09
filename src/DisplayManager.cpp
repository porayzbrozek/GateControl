#include "DisplayManager.h"
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "Config.h"
#include <WiFi.h> 

void updateDisplay() {
  // zabezpieczenie przed zbyt czestym odswiezaniem oleda
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 100) return;  
  lastUpdate = millis();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  long rssi = WiFi.RSSI();
  int quality = map(rssi, -100, -50, 0, 100);
  quality = constrain(quality, 0, 100);

  display.setCursor(0, 0);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi: OK  sygnal:");
    display.print(quality);
    display.println("%");
  } else {
    display.println("WiFi: Brak");
  }

  display.setCursor(0, 10);
  display.print("SinricPro: ");
  display.println(sinricProConnected ? "OK" : "Brak");

  display.setCursor(0, 20);
  display.print("Brama: ");
  if (motorRunning) {
    display.println(isGateOpen ? "Zamykanie..." : "Otwieranie...");
  } else {
    display.println(isGateOpen ? "Otwarta" : "Zamknieta");
  }

  display.setCursor(0, 30);
  display.print("Pozostało:");
  int currentCount;
  portENTER_CRITICAL(&mux);
  currentCount = abs(encoderPulseCount);
  portEXIT_CRITICAL(&mux);
  int remaining = max(0, adjustedPulseLimit - currentCount);
  display.println(remaining);

  display.setCursor(0, 40);
  display.print("impulsy manualne:");
  display.println(manualMovementPulses);

  int progress = map(abs(encoderPulseCount), 0, adjustedPulseLimit, 0, 118);
  display.drawRect(5, 52, 118, 10, SSD1306_WHITE);
  display.fillRect(6, 53, progress, 8, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  const char* label = "Poray_Electrics";
  int textX = (SCREEN_WIDTH - strlen(label) * 6) / 2;
  int textY = 54;

  for (int i = 0; i < strlen(label); i++) {
    int charX = textX + i * 6;
    if (charX + 6 < 6 + progress) {
      display.setCursor(charX, textY);
      display.write(label[i]);
    }
  }

  display.display();
}
