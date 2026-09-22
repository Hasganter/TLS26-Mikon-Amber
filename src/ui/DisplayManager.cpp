#include "DisplayManager.h"

DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    isDirty(true),
    terhubung(false),
    oledAddress(SCREEN_ADDRESS),
    waktuRenderTerakhir(0),
    waktuDetikTerakhir(0),
    waktuProbeTerakhir(0) {}

void DisplayManager::scanI2C() {
  Serial.println("[I2C SCAN] Memindai bus I2C (SDA=GPIO21, SCL=GPIO22)...");
  int count = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("[I2C SCAN] Peranti I2C terdeteksi pada alamat 0x%02X!\r\n", addr);
      count++;
    }
  }
  if (count == 0) {
    Serial.println("[I2C SCAN] PERINGATAN: Tidak ada peranti I2C terdeteksi!");
    Serial.println("[I2C SCAN] Periksa: 1) Kabel SDA (D21) & SCL (D22). 2) Daya VCC (3.3V/5V) & GND.");
  }
}

bool DisplayManager::inisialisasi() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  Wire.setClock(400000);

  // 1. Coba alamat konfigurasi utama (0x3C)
  oledAddress = SCREEN_ADDRESS;
  terhubung = display.begin(SSD1306_SWITCHCAPVCC, oledAddress);

  // 2. Jika gagal, coba alamat alternatif (0x3D)
  if (!terhubung) {
    oledAddress = (SCREEN_ADDRESS == 0x3C) ? 0x3D : 0x3C;
    terhubung = display.begin(SSD1306_SWITCHCAPVCC, oledAddress);
  }

  if (!terhubung) {
    Serial.println("[DISPLAY] Gagal inisialisasi OLED pada 0x3C maupun 0x3D!");
    scanI2C();
    return false;
  }

  Serial.printf("[DISPLAY] OLED SSD1306 berhasil terhubung pada alamat 0x%02X\r\n", oledAddress);
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 24);
  display.print("TLS26 MIKON AMBER");
  display.setCursor(26, 36);
  display.print("Parking System");
  display.display();
  delay(1000);
  isDirty = true;
  return true;
}

bool DisplayManager::isOk() const {
  return terhubung;
}

void DisplayManager::markDirty() {
  isDirty = true;
}

void DisplayManager::render(const UIState &state, const WiFiManager &wifi) {
  unsigned long sekarang = millis();

  // Jika OLED belum terhubung, coba deteksi & inisialisasi ulang tiap 2 detik
  if (!terhubung) {
    if (sekarang - waktuProbeTerakhir >= 2000) {
      waktuProbeTerakhir = sekarang;
      Wire.beginTransmission(oledAddress);
      if (Wire.endTransmission() == 0) {
        terhubung = display.begin(SSD1306_SWITCHCAPVCC, oledAddress);
        if (terhubung) {
          Serial.println("[DISPLAY] OLED berhasil tersambung kembali!");
          isDirty = true;
        }
      }
    }
    return;
  }

  // Throttling: Cek apakah perlu render ulang (hemat CPU & I2C)
  // 1. Jika ada flag dirty (input baru, tombol ditekan, status berubah)
  // 2. Atau setiap 500ms untuk clock update / status refresh
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

    case STATUS_DEBUG_KOMPONEN:
      UIViewDebug::renderKomponen(display, state);
      break;
  }

  display.display();
}
