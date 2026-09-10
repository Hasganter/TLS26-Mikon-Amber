#include "UIViewParking.h"

void UIViewParking::renderStandby(Adafruit_SSD1306 &display, const UIState &state) {
  if (state.slotTersedia <= 0) {
    // 1. Kasus Parkir Penuh
    UIHelper::drawTextScroll(display, 2, "! PARKIR PENUH !", 1, true);

    display.setTextSize(3);
    display.setCursor(19, 18);
    display.print("PENUH");

    display.drawFastHLine(14, 14, 100, SSD1306_WHITE);
    display.drawFastHLine(14, 45, 100, SSD1306_WHITE);

    UIHelper::drawTextScroll(display, 52, "Gerbang Dikunci", 1, true);
  } else if (state.jarakKiri < AMBANG_DETEKSI_CM) {
    // 2. Kasus Mobil Mendekat di Lane Masuk
    UIHelper::drawTextScroll(display, 2, "KENDARAAN MASUK", 1, true);

    display.setTextSize(2);
    display.setCursor(34, 22);
    display.print("TIKET");

    display.drawFastHLine(14, 14, 100, SSD1306_WHITE);
    display.drawFastHLine(14, 45, 100, SSD1306_WHITE);

    UIHelper::drawTextScroll(display, 52, "Tekan Tombol Apapun", 1, true);
  } else {
    // 3. Kondisi Standby Normal
    if (state.layarStandbyModeA) {
      UIHelper::drawTextScroll(display, 2, "SELAMAT DATANG", 1, true);
    } else {
      UIHelper::drawTextScroll(display, 2, "STATUS PARKIR", 1, true);
    }

    // Tampilan Angka Slot Besar di Tengah (Auto-Scaling hingga 999)
    String slotStr = String(state.slotTersedia) + "/" + String(state.kapasitasMaksimal);
    int charCount = slotStr.length();

    if (charCount <= 5) {
      // Font Size 3 (lebar 18px per karakter)
      int totalWidth = charCount * 18;
      int16_t xPos = (SCREEN_WIDTH - totalWidth) / 2;
      display.setTextSize(3);
      display.setCursor(xPos, 18);
      display.print(slotStr);
    } else {
      // Font Size 2 jika karakter > 5 (misal 150/999, lebar 12px per karakter)
      int totalWidth = charCount * 12;
      int16_t xPos = (SCREEN_WIDTH - totalWidth) / 2;
      display.setTextSize(2);
      display.setCursor(xPos, 22);
      display.print(slotStr);
    }

    display.drawFastHLine(14, 14, 100, SSD1306_WHITE);
    display.drawFastHLine(14, 45, 100, SSD1306_WHITE);

    // Baris Bawah
    if (state.layarStandbyModeA) {
      UIHelper::drawTextScroll(display, 52, "Tekan Tombol Tiket", 1, true);
    } else {
      String strWaktu = String(state.bufHari) + ", " + state.bufWaktu;
      UIHelper::drawTextScroll(display, 52, strWaktu, 1, true);
    }
  }
}

void UIViewParking::renderCountdown(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ PROSES MASUK ]");

  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("Membuka gate dlm:");

  unsigned long sisaDetik = (state.sisaCountdown5s + 999) / 1000;
  display.setTextSize(2);
  display.setCursor(54, 30);
  display.print(sisaDetik);
  display.print("s");

  UIHelper::drawFooter(display, "Tekan C: Batalkan", "");
}

void UIViewParking::renderGateOpen(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ PORTAL TERBUKA ]");

  display.setTextSize(1);
  if (state.mobilSedangDiBawahGate) {
    display.setCursor(10, 16);
    display.print("Mobil Sedang Lewat");

    display.setTextSize(2);
    display.setCursor(34, 32);
    display.print("MASUK");

    UIHelper::drawFooter(display, "Tahan: Mobil Melintas", "");
  } else if (state.mobilPernahTerdeteksiDiGate) {
    display.setCursor(6, 16);
    display.print("Mobil Selesai Lewat");

    unsigned long sisaDetik = (state.sisaTutupAmanMs + 999) / 1000;
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.print("Tutup Gate dalam: ");
    display.print(sisaDetik);
    display.print("s");

    UIHelper::drawFooter(display, "Menutup Otomatis...", "");
  } else {
    display.setCursor(20, 16);
    display.print("Silakan Masuk");

    unsigned long sisaDetik = (state.sisaTimeoutGateMs + 999) / 1000;
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.print("Timeout Gate: ");
    display.print(sisaDetik);
    display.print("s");

    UIHelper::drawFooter(display, "Maju melewati gate", "");
  }
}

void UIViewParking::renderLaneKeluar(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ LANE KELUAR ]");

  display.setTextSize(2);
  display.setCursor(28, 18);
  display.print("SAMPAI");
  display.setCursor(16, 34);
  display.print("JUMPA!");

  UIHelper::drawTextScroll(display, 52, "Hati-hati di jalan!", 1, true);
}
