#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../config/SystemTypes.h"
#include "../config/Config.h"
#include "../network/WiFiManager.h"
#include "UIHelper.h"

class UIViewDebug {
public:
  static void renderMenu(Adafruit_SSD1306 &display, const UIState &state);
  static void renderWaktu(Adafruit_SSD1306 &display, const UIState &state);
  static void renderTanggal(Adafruit_SSD1306 &display, const UIState &state);
  static void renderSlot(Adafruit_SSD1306 &display, const UIState &state);
  static void renderWiFiScan(Adafruit_SSD1306 &display, const UIState &state, const WiFiManager &wifi);
  static void renderWiFiPassword(Adafruit_SSD1306 &display, const UIState &state);
  static void renderWiFiManual(Adafruit_SSD1306 &display, const UIState &state);
  static void renderWiFiStatus(Adafruit_SSD1306 &display, const UIState &state);
};

