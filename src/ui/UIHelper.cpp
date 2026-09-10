#include "UIHelper.h"

void UIHelper::drawTextScroll(
  Adafruit_SSD1306 &display,
  int16_t y,
  const String &text,
  uint8_t textSize,
  bool centerIfFits
) {
  display.setTextSize(textSize);
  display.setTextWrap(false);

  int textPixelWidth = text.length() * 6 * textSize;

  if (textPixelWidth <= SCREEN_WIDTH) {
    int16_t x = centerIfFits ? ((SCREEN_WIDTH - textPixelWidth) / 2) : 0;
    display.setCursor(x, y);
    display.print(text);
  } else {
    // Auto-scroll horizontal halus dengan jeda di awal dan akhir
    const unsigned long PAUSE_AWAL_MS = 1200;
    const unsigned long PAUSE_AKHIR_MS = 1000;
    const unsigned long MS_PER_PIXEL = 35;
    const int PADDING_AKHIR = 20;

    int totalScroll = (textPixelWidth - SCREEN_WIDTH) + PADDING_AKHIR;
    unsigned long durasiScroll = totalScroll * MS_PER_PIXEL;
    unsigned long totalSiklus = PAUSE_AWAL_MS + durasiScroll + PAUSE_AKHIR_MS;

    unsigned long fase = millis() % totalSiklus;
    int16_t xOffset = 0;

    if (fase < PAUSE_AWAL_MS) {
      xOffset = 0;
    } else if (fase < PAUSE_AWAL_MS + durasiScroll) {
      xOffset = (int16_t)((fase - PAUSE_AWAL_MS) / MS_PER_PIXEL);
    } else {
      xOffset = totalScroll;
    }

    display.setCursor(-xOffset, y);
    display.print(text);
  }
}

void UIHelper::drawHeader(Adafruit_SSD1306 &display, const char* title) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(title);
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
}

void UIHelper::drawFooter(Adafruit_SSD1306 &display, const char* leftGuide, const char* rightGuide) {
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(leftGuide);
  if (rightGuide && strlen(rightGuide) > 0) {
    int rightLen = strlen(rightGuide) * 6;
    display.setCursor(SCREEN_WIDTH - rightLen, 56);
    display.print(rightGuide);
  }
}
