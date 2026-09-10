#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../config/Config.h"

class UIHelper {
public:
  // Render teks dengan animasi scroll marquee jika panjang > 128px
  static void drawTextScroll(
    Adafruit_SSD1306 &display,
    int16_t y,
    const String &text,
    uint8_t textSize = 1,
    bool centerIfFits = true
  );

  // Gambar header layar dengan garis pemisah horizontal
  static void drawHeader(Adafruit_SSD1306 &display, const char* title);

  // Gambar panduan tombol di baris bawah layar
  static void drawFooter(Adafruit_SSD1306 &display, const char* leftGuide, const char* rightGuide);
};

