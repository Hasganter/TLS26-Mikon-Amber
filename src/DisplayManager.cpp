#include "include/DisplayManager.h"

DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

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
  return true;
}

void DisplayManager::drawTextScroll(int16_t y, const String &text, uint8_t textSize, bool centerIfFits) {
  display.setTextSize(textSize);
  display.setTextWrap(false);

  int textPixelWidth = text.length() * 6 * textSize;

  if (textPixelWidth <= SCREEN_WIDTH) {
    int16_t x = centerIfFits ? ((SCREEN_WIDTH - textPixelWidth) / 2) : 0;
    display.setCursor(x, y);
    display.print(text);
  } else {
    // Auto-scroll horizontal halus dengan pause di awal dan akhir
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

void DisplayManager::render(
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
  const String &pesanFeedbackDebug,
  bool fokusMaksimal,
  const String &bufferTersedia,
  const String &bufferMaksimal
) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  switch (status) {
    // ─────────────────────────────────────────────────────────
    // STATUS_STANDBY: Tampilan Dashboard Bersih (Slot di Tengah)
    // ─────────────────────────────────────────────────────────
    case STATUS_STANDBY: {
      if (slotTersedia <= 0) {
        // Status Parkir Penuh
        drawTextScroll(2, "! PARKIR PENUH !", 1, true);

        // Angka tengah Size 3: "PENUH"
        display.setTextSize(3);
        display.setCursor(19, 18);
        display.print("PENUH");

        display.drawFastHLine(14, 14, 100, SSD1306_WHITE);
        display.drawFastHLine(14, 45, 100, SSD1306_WHITE);

        drawTextScroll(52, "Gerbang Dikunci", 1, true);
      } else if (jarakKiri < AMBANG_DETEKSI_CM) {
        // Ada kendaraan terdeteksi di sensor masuk
        drawTextScroll(2, "KENDARAAN MASUK", 1, true);

        display.setTextSize(2);
        display.setCursor(34, 16);
        display.print("TEKAN");
        display.setCursor(34, 32);
        display.print("TIKET");

        drawTextScroll(52, "Tekan Tombol Tiket", 1, true);
      } else {
        // Standby Normal
        if (layarStandbyModeA) {
          drawTextScroll(2, "SELAMAT DATANG", 1, true);
        } else {
          drawTextScroll(2, "STATUS PARKIR", 1, true);
        }

        // Kartu Slot Tengah (Font Size 3 Besar)
        display.drawFastHLine(14, 14, 100, SSD1306_WHITE);
        display.drawFastHLine(14, 45, 100, SSD1306_WHITE);

        String strSlot = String(slotTersedia) + "/" + String(kapasitasMaksimal);
        int strWidth = strSlot.length() * 18;
        int16_t xCenter = (SCREEN_WIDTH - strWidth) / 2;
        if (xCenter < 0) xCenter = 0;

        display.setTextSize(3);
        display.setCursor(xCenter, 18);
        display.print(strSlot);

        // Baris Bawah
        if (layarStandbyModeA) {
          drawTextScroll(52, "Tekan Tombol Tiket", 1, true);
        } else {
          String strWaktuBawah = String(bufHari) + ", " + String(bufWaktu);
          drawTextScroll(52, strWaktuBawah, 1, true);
        }
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_COUNTDOWN_5S: Jendela Pembatalan & Hitung Mundur
    // ─────────────────────────────────────────────────────────
    case STATUS_COUNTDOWN_5S: {
      int sisaDetik = (sisaCountdown5s / 1000) + 1;
      if (sisaDetik > 5) sisaDetik = 5;
      if (sisaDetik < 1) sisaDetik = 1;

      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ PROSES MASUK ]");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      display.setCursor(0, 16);
      display.print("Membuka gate dalam:");

      display.setTextSize(3);
      display.setCursor(54, 26);
      display.print(sisaDetik);

      drawTextScroll(52, "Tekan C : Batalkan", 1, true);
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_GATE_TERBUKA: Portal Masuk & Safety Hold
    // ─────────────────────────────────────────────────────────
    case STATUS_GATE_TERBUKA: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ PORTAL TERBUKA ]");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      if (mobilSedangDiBawahGate) {
        display.setCursor(0, 16);
        display.print("Mobil Sedang Lewat");

        display.setTextSize(2);
        display.setCursor(34, 32);
        display.print("MASUK");

        drawTextScroll(52, "Safety Hold Aktif", 1, true);
      } else if (mobilPernahTerdeteksiDiGate) {
        int sisaDetik = (sisaTutupAmanMs / 1000) + 1;
        if (sisaDetik < 1) sisaDetik = 1;

        display.setCursor(0, 16);
        display.print("Mobil Selesai Lewat");
        display.setCursor(0, 30);
        display.print("Tutup Gate dalam:");

        display.setTextSize(2);
        display.setCursor(54, 44);
        display.print(sisaDetik);
        display.print("s");
      } else {
        int sisaDetik = (sisaTimeoutGateMs / 1000) + 1;
        if (sisaDetik < 1) sisaDetik = 1;

        display.setCursor(0, 16);
        display.print("Silakan Lewat...");
        display.setCursor(0, 30);
        display.print("Batas Waktu: ");
        display.print(sisaDetik);
        display.print("s");
        display.setCursor(0, 50);
        display.print("Safety Gate Aktif");
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_LANE_KELUAR: Lane Keluar & Salam Perpisahan
    // ─────────────────────────────────────────────────────────
    case STATUS_LANE_KELUAR: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ LANE KELUAR ]");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      display.setTextSize(2);
      display.setCursor(28, 18);
      display.print("SAMPAI");
      display.setCursor(16, 34);
      display.print("JUMPA!");

      drawTextScroll(52, "Hati-hati di jalan!", 1, true);
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_MENU: Menu Utama Konfigurasi
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_MENU: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("--- DEBUG MODE ---");
      display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

      display.setCursor(0, 14);
      display.print("1: Waktu  (HHMM)");
      display.setCursor(0, 26);
      display.print("2: Tanggal(DDMMYYYY)");
      display.setCursor(0, 38);
      display.print("3: Slot Parkir");
      display.setCursor(0, 50);
      display.print("C: Keluar ke Standby");
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_WAKTU: Sub-menu Edit Waktu (HHMM)
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_WAKTU: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ SET WAKTU (24h) ]");
      display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

      if (pesanFeedbackDebug.length() > 0) {
        drawTextScroll(28, pesanFeedbackDebug, 1, true);
      } else {
        display.setCursor(0, 14);
        display.print("Jam skrg: ");
        display.print(bufWaktu);

        display.setCursor(0, 26);
        display.print("Format  : HHMM");

        display.setTextSize(2);
        display.setCursor(20, 38);
        display.print("[");
        display.print(bufferInputDebug);
        for (int i = bufferInputDebug.length(); i < 4; i++) display.print("_");
        display.print("]");

        display.setTextSize(1);
        display.setCursor(0, 56);
        display.print("*:Hapus #:Ok C:Batal");
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_TANGGAL: Sub-menu Edit Tanggal (DDMMYYYY)
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_TANGGAL: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ SET TANGGAL ]");
      display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

      if (pesanFeedbackDebug.length() > 0) {
        drawTextScroll(28, pesanFeedbackDebug, 1, true);
      } else {
        display.setCursor(0, 14);
        display.print("Tgl skrg: ");
        display.print(bufTanggal);

        display.setCursor(0, 26);
        display.print("Format  : DDMMYYYY");

        display.setCursor(8, 40);
        display.print("Input: [");
        display.print(bufferInputDebug);
        for (int i = bufferInputDebug.length(); i < 8; i++) display.print("_");
        display.print("]");

        display.setCursor(0, 56);
        display.print("*:Hapus #:Ok C:Batal");
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_SLOT: Sub-menu Edit Slot Parkir Terpadu
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_SLOT: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ ATUR SLOT PARKIR ]");
      display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

      if (pesanFeedbackDebug.length() > 0) {
        drawTextScroll(28, pesanFeedbackDebug, 1, true);
      } else {
        // Baris 1: Slot Tersedia
        display.setCursor(0, 14);
        display.print(!fokusMaksimal ? ">" : " ");
        display.print(" Tersedia: [");
        if (!fokusMaksimal && bufferTersedia.length() > 0) {
          display.print(bufferTersedia);
        } else {
          display.print(slotTersedia);
        }
        display.print("]");

        // Baris 2: Kapasitas Maksimal
        display.setCursor(0, 26);
        display.print(fokusMaksimal ? ">" : " ");
        display.print(" Maksimal: [");
        if (fokusMaksimal && bufferMaksimal.length() > 0) {
          display.print(bufferMaksimal);
        } else {
          display.print(kapasitasMaksimal);
        }
        display.print("]");

        // Baris 3 & 4: Panduan Tombol Ringkas
        display.setCursor(0, 40);
        display.print("A:Pilih Field  *:Del");

        display.setCursor(0, 52);
        display.print("#:Simpan       C:Batal");
      }
      break;
    }
  }

  display.display();
}
