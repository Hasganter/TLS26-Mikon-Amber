#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"

class DisplayManager {
public:
  DisplayManager();

  // Inisialisasi layar OLED dan tampilkan splash screen
  bool inisialisasi();

  // Render antarmuka visual sesuai status sistem saat ini
  void render(
    StatusSistem status,
    int slotTersedia,
    int kapasitasMaksimal,
    bool layarStandbyModeA,
    float jarakKiri,
    unsigned long sisaCountdown5s,
    bool mobilSedangDiBawahGate,
    bool mobilPernahTerdeteksiDiGate,
    unsigned long sisaTutupAmanMs,
    unsigned long sisaTimeoutGateMs,
    const char* bufWaktu,
    const char* bufTanggal,
    const char* bufHari,
    const String &bufferInputDebug,
    const String &pesanFeedbackDebug
  );

private:
  Adafruit_SSD1306 display;
};
