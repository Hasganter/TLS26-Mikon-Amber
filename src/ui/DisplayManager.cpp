#include "DisplayManager.h"

DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    isDirty(true),
    waktuRenderTerakhir(0),
    waktuDetikTerakhir(0) {}

bool DisplayManager::inisialisasi() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    return false;
  }
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 24);
  display.print("TLS26 MIKON AMBER");
  display.setCursor(20, 36);
  display.print("Dual Lane System");
  display.display();
  delay(1000);
  isDirty = true;
  return true;
}

void DisplayManager::markDirty() {
  isDirty = true;
}

void DisplayManager::render(const UIState &state, const WiFiManager &wifi) {
  unsigned long sekarang = millis();

  // Throttling: Cek apakah perlu render ulang (hemat CPU & I2C)
  // 1. Jika ada flag dirty (input baru, tombol ditekan, status berubah)
  // 2. Atau setiap 500ms untuk clock update
  // 3. Atau setiap 33ms jika sedang di mode standby (animasi marquee)
  bool butuhRender = isDirty;

  if (sekarang - waktuDetikTerakhir >= 500) {
    waktuDetikTerakhir = sekarang;
    butuhRender = true;
  }

  // Jika ada animasi teks berjalan (pada mode standby atau countdown), atau sedang scan/connecting WiFi
  if (state.status == STATUS_STANDBY || state.status == STATUS_LANE_KELUAR || state.isScanningWiFi || state.wifiStatus == WIFI_STATUS_CONNECTING) {
    if (sekarang - waktuRenderTerakhir >= 35) {
      butuhRender = true;
    }
  }

  // Batasi refresh rate maksimum ~30 FPS (33ms)
  if (!butuhRender || (sekarang - waktuRenderTerakhir < 33)) {
    return;
  }

  waktuRenderTerakhir = sekarang;
  isDirty = false;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  switch (state.status) {
    case STATUS_STANDBY:
      UIViewParking::renderStandby(display, state);
      break;

    case STATUS_COUNTDOWN_5S:
      UIViewParking::renderCountdown(display, state);
      break;

    case STATUS_GATE_TERBUKA:
      UIViewParking::renderGateOpen(display, state);
      break;

    case STATUS_LANE_KELUAR:
      UIViewParking::renderLaneKeluar(display, state);
      break;

    case STATUS_DEBUG_MENU:
      UIViewDebug::renderMenu(display, state);
      break;

    case STATUS_DEBUG_WAKTU:
      UIViewDebug::renderWaktu(display, state);
      break;

    case STATUS_DEBUG_TANGGAL:
      UIViewDebug::renderTanggal(display, state);
      break;

    case STATUS_DEBUG_SLOT:
      UIViewDebug::renderSlot(display, state);
      break;

    case STATUS_DEBUG_WIFI_SCAN:
      UIViewDebug::renderWiFiScan(display, state, wifi);
      break;

    case STATUS_DEBUG_WIFI_PASS:
      if (state.wifiStatus != WIFI_STATUS_DISCONNECTED) {
        UIViewDebug::renderWiFiStatus(display, state);
      } else {
        UIViewDebug::renderWiFiPassword(display, state);
      }
      break;

    case STATUS_DEBUG_WIFI_MANUAL:
      UIViewDebug::renderWiFiManual(display, state);
      break;
  }

  display.display();
}
