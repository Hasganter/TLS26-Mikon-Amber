#ifndef UI_VIEW_PARKING_H
#define UI_VIEW_PARKING_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../config/SystemTypes.h"
#include "../config/Config.h"
#include "UIHelper.h"

class UIViewParking {
public:
  static void renderStandby(Adafruit_SSD1306 &display, const UIState &state);
  static void renderCountdown(Adafruit_SSD1306 &display, const UIState &state);
  static void renderGateOpen(Adafruit_SSD1306 &display, const UIState &state);
  static void renderLaneKeluar(Adafruit_SSD1306 &display, const UIState &state);
};

#endif // UI_VIEW_PARKING_H
