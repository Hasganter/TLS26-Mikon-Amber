#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../config/SystemTypes.h"
#include "../config/Config.h"
#include "../network/WiFiManager.h"
#include "UIHelper.h"
#include "UIViewParking.h"
#include "UIViewDebug.h"

class DisplayManager {
public:
  DisplayManager();

  // Inisialisasi layar OLED dan tampilkan splash screen
  bool inisialisasi();

  // Render antarmuka visual sesuai status sistem saat ini
  void render(const UIState &state, const WiFiManager &wifi);

  // Tandai bahwa tampilan perlu segera diperbarui (dirty flag)
  void markDirty();

private:
  Adafruit_SSD1306 display;
  bool isDirty;
  unsigned long waktuRenderTerakhir;
  unsigned long waktuDetikTerakhir;
};

#endif // DISPLAY_MANAGER_H
