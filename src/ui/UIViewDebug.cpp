#include "UIViewDebug.h"

void UIViewDebug::renderMenu(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "--- DEBUG MODE ---");

  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print("1: Waktu  (HHMMSS)");
  display.setCursor(0, 24);
  display.print("2: Tanggal(DDMMYYYY)");
  display.setCursor(0, 34);
  display.print("3: Slot Parkir");
  display.setCursor(0, 44);
  display.print("4: Koneksi WiFi");
  display.setCursor(0, 54);
  display.print("C: Keluar ke Standby");
}

void UIViewDebug::renderWaktu(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ SET WAKTU 24H ]");

  if (state.pesanFeedbackDebug.length() > 0) {
    UIHelper::drawTextScroll(display, 28, state.pesanFeedbackDebug, 1, true);
  } else {
    display.setTextSize(1);
    display.setCursor(0, 14);
    display.print("Jam skrg: ");
    display.print(state.bufWaktu);

    display.setCursor(0, 26);
    display.print("Format  : HHMMSS");

    // Format tampilan buffer [HH:MM:SS]
    display.setCursor(4, 40);
    display.print("Input: [");
    String raw = state.bufferInputDebug;
    for (int i = 0; i < 6; i++) {
      if (i < raw.length()) {
        display.print(raw[i]);
      } else {
        display.print("_");
      }
      if (i == 1 || i == 3) display.print(":");
    }
    display.print("]");

    UIHelper::drawFooter(display, "*:Hapus #:Ok", "C:Batal");
  }
}

void UIViewDebug::renderTanggal(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ SET TANGGAL ]");

  if (state.pesanFeedbackDebug.length() > 0) {
    UIHelper::drawTextScroll(display, 28, state.pesanFeedbackDebug, 1, true);
  } else {
    display.setTextSize(1);
    display.setCursor(0, 14);
    display.print("Tgl skrg: ");
    display.print(state.bufTanggal);

    display.setCursor(0, 26);
    display.print("Format  : DDMMYYYY");

    display.setCursor(4, 40);
    display.print("Input: [");
    display.print(state.bufferInputDebug);
    for (int i = state.bufferInputDebug.length(); i < 8; i++) {
      display.print("_");
    }
    display.print("]");

    UIHelper::drawFooter(display, "*:Hapus #:Ok", "C:Batal");
  }
}

void UIViewDebug::renderSlot(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ ATUR SLOT PARKIR ]");

  if (state.pesanFeedbackDebug.length() > 0) {
    UIHelper::drawTextScroll(display, 28, state.pesanFeedbackDebug, 1, true);
  } else {
    display.setTextSize(1);

    // Baris 1: Slot Tersedia (Menampilkan nilai sebelumnya dan nilai baru)
    display.setCursor(0, 14);
    display.print(!state.fokusMaksimal ? ">" : " ");
    display.print("Tersedia: ");
    display.print(state.slotTersedia);
    display.print(" -> [");
    if (!state.fokusMaksimal && state.bufferTersedia.length() > 0) {
      display.print(state.bufferTersedia);
    } else {
      display.print("-");
    }
    display.print("]");

    // Baris 2: Kapasitas Maksimal (Menampilkan nilai sebelumnya dan nilai baru)
    display.setCursor(0, 26);
    display.print(state.fokusMaksimal ? ">" : " ");
    display.print("Maksimal: ");
    display.print(state.kapasitasMaksimal);
    display.print(" -> [");
    if (state.fokusMaksimal && state.bufferMaksimal.length() > 0) {
      display.print(state.bufferMaksimal);
    } else {
      display.print("-");
    }
    display.print("]");

    // Baris 3 & 4: Panduan Tombol
    display.setCursor(0, 40);
    display.print("A:Pilih Field  *:Del");

    display.setCursor(0, 52);
    display.print("#:Simpan       C:Batal");
  }
}

void UIViewDebug::renderWiFiScan(Adafruit_SSD1306 &display, const UIState &state, const WiFiManager &wifi) {
  int totalItems = wifi.getJumlahJaringan() + 1; // +1 untuk opsi [+ Input Manual]
  int currentItem = state.wifiSelectedIndex + 1;

  char titleBuf[32];
  sprintf(titleBuf, "[ PILIH WIFI (%d/%d) ]", currentItem, totalItems);
  UIHelper::drawHeader(display, titleBuf);

  if (state.isScanningWiFi) {
    display.setTextSize(1);
    display.setCursor(8, 20);
    display.print("Memindai Jaringan...");
    display.setCursor(16, 34);
    display.print("Mohon tunggu...");
    UIHelper::drawFooter(display, "C: Batalkan", "");
    return;
  }

  // Tampilkan 3 item sekaligus berdasarkan scroll offset
  int startIdx = state.wifiScrollOffset;
  for (int row = 0; row < 3; row++) {
    int itemIdx = startIdx + row;
    if (itemIdx >= totalItems) break;

    int16_t y = 14 + (row * 12);
    display.setCursor(0, y);
    display.setTextSize(1);

    bool isSelected = (itemIdx == state.wifiSelectedIndex);
    display.print(isSelected ? ">" : " ");

    if (itemIdx == 0) {
      display.print("[+ Input Manual]");
    } else {
      int netIdx = itemIdx - 1;
      String ssid = wifi.getSSID(netIdx);
      if (ssid.length() > 14) {
        ssid = ssid.substring(0, 13) + "~";
      }
      display.print(ssid);
      if (wifi.isEncrypted(netIdx)) {
        display.print("*");
      }
    }
  }

  UIHelper::drawFooter(display, "A:Up B:Dn #:Pilih", "*:Scan C:Batal");
}

void UIViewDebug::renderWiFiPassword(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ PASSWORD WIFI ]");

  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print("SSID: ");
  display.print(state.wifiTargetSSID);

  display.setCursor(0, 26);
  display.print("Pass: [");
  display.print(state.t9DisplayText);
  display.print("]");

  display.setCursor(0, 40);
  display.print("Mode: [");
  display.print(state.t9ModeStr);
  display.print("] B:Spasi");

  UIHelper::drawFooter(display, "*:Del D:Mode", "#:Konek C:Batal");
}

void UIViewDebug::renderWiFiManual(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ INPUT SSID MANUAL ]");

  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print("Ketik Nama Jaringan:");

  display.setCursor(0, 26);
  display.print("SSID: [");
  display.print(state.t9DisplayText);
  display.print("]");

  display.setCursor(0, 40);
  display.print("Mode: [");
  display.print(state.t9ModeStr);
  display.print("] B:Spasi");

  UIHelper::drawFooter(display, "*:Del D:Mode", "#:Lanjut C:Batal");
}

void UIViewDebug::renderWiFiStatus(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ STATUS KONEKSI ]");

  display.setTextSize(1);
  if (state.wifiStatus == WIFI_STATUS_CONNECTING) {
    display.setCursor(0, 16);
    display.print("Menghubungkan ke:");
    display.setCursor(0, 28);
    display.print(state.wifiTargetSSID);

    display.setCursor(0, 42);
    display.print("Mohon tunggu...");
    UIHelper::drawFooter(display, "C: Batalkan", "");
  } else if (state.wifiStatus == WIFI_STATUS_CONNECTED) {
    display.setCursor(10, 16);
    display.print("WIFI TERHUBUNG!");

    display.setCursor(0, 30);
    display.print("IP: ");
    display.print(state.wifiAssignedIP);

    display.setCursor(0, 44);
    display.print("Disimpan ke Flash NVS");
    UIHelper::drawFooter(display, "#: Selesai", "");
  } else {
    display.setCursor(14, 16);
    display.print("KONEKSI GAGAL!");

    display.setCursor(0, 30);
    display.print("Periksa Password /");
    display.setCursor(0, 42);
    display.print("Jangkauan Sinyal");
    UIHelper::drawFooter(display, "*: Ulangi", "C: Batal");
  }
}
