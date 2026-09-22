#include "UIViewDebug.h"

void UIViewDebug::renderMenu(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "--- DEBUG MODE ---");

  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print("1:Waktu    2:Tanggal");
  display.setCursor(0, 24);
  display.print("3:Slot     4:WiFi");
  display.setCursor(0, 34);
  display.print("5:Status Komponen");
  display.setCursor(0, 44);
  display.print("6:Mute Notif: ");
  display.print(state.muteMissingNotifier ? "[ON]" : "[OFF]");
  display.setCursor(0, 54);
  display.print("C:Keluar ke Standby");
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
    display.print("Slot: ");
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
    display.print("Maks: ");
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
    display.print("#:Simpan     C:Batal");
  }
}

void UIViewDebug::renderWiFiScan(Adafruit_SSD1306 &display, const UIState &state, const WiFiManager &wifi) {
  int totalItems = wifi.getJumlahJaringan() + 1; // +1 untuk opsi [+ Input Manual]
  int currentItem = state.wifiSelectedIndex + 1;

  char titleBuf[32];
  sprintf(titleBuf, "[ WIFI %d/%d (A/B) ]", currentItem, totalItems);
  UIHelper::drawHeader(display, titleBuf);

  if (state.isScanningWiFi) {
    display.setTextSize(1);
    display.setCursor(16, 20);
    display.print("Memindai WiFi...");
    display.setCursor(19, 34);
    display.print("Mohon tunggu...");
    UIHelper::drawFooter(display, "C: Batal", "");
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

  UIHelper::drawFooter(display, "#:Pilih *:Scan", "C:Batal");
}

void UIViewDebug::renderWiFiPassword(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ PASSWORD WIFI ]");

  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print("SSID: ");
  String showSSID = state.wifiTargetSSID;
  if (showSSID.length() > 14) {
    showSSID = showSSID.substring(0, 13) + "~";
  }
  display.print(showSSID);

  display.setCursor(0, 26);
  display.print("Pass: [");
  String showPass = state.t9DisplayText;
  if (showPass.length() > 12) {
    showPass = "~" + showPass.substring(showPass.length() - 11);
  }
  display.print(showPass);
  display.print("]");

  display.setCursor(0, 40);
  display.print("Mode:[");
  display.print(state.t9ModeStr);
  display.print("] B:Spasi");

  UIHelper::drawFooter(display, "*:Del D:Md", "#:Ok C:Btl");
}

void UIViewDebug::renderWiFiManual(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ INPUT SSID ]");

  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print("Ketik Nama Jaringan:");

  display.setCursor(0, 26);
  display.print("SSID: [");
  String showSSID = state.t9DisplayText;
  if (showSSID.length() > 12) {
    showSSID = "~" + showSSID.substring(showSSID.length() - 11);
  }
  display.print(showSSID);
  display.print("]");

  display.setCursor(0, 40);
  display.print("Mode:[");
  display.print(state.t9ModeStr);
  display.print("] B:Spasi");

  UIHelper::drawFooter(display, "*:Del D:Md", "#:Ok C:Btl");
}

void UIViewDebug::renderWiFiStatus(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ STATUS KONEKSI ]");

  display.setTextSize(1);
  if (state.wifiStatus == WIFI_STATUS_CONNECTING) {
    display.setCursor(0, 16);
    display.print("Menghubungkan ke:");
    display.setCursor(0, 28);
    String showSSID = state.wifiTargetSSID;
    if (showSSID.length() > 20) {
      showSSID = showSSID.substring(0, 19) + "~";
    }
    display.print(showSSID);

    display.setCursor(0, 42);
    display.print("Mohon tunggu...");
    UIHelper::drawFooter(display, "C: Batal", "");
  } else if (state.wifiStatus == WIFI_STATUS_CONNECTED) {
    display.setCursor(16, 16);
    display.print("WIFI TERHUBUNG!");

    display.setCursor(0, 30);
    display.print("IP: ");
    display.print(state.wifiAssignedIP);

    display.setCursor(10, 44);
    display.print("Tersimpan di Flash");
    UIHelper::drawFooter(display, "#: Selesai", "");
  } else {
    display.setCursor(20, 16);
    display.print("KONEKSI GAGAL!");

    display.setCursor(0, 30);
    display.print("Periksa Password /");
    display.setCursor(0, 42);
    display.print("Jangkauan Sinyal");
    UIHelper::drawFooter(display, "*: Ulangi", "C: Batal");
  }
}

void UIViewDebug::renderKomponen(Adafruit_SSD1306 &display, const UIState &state) {
  UIHelper::drawHeader(display, "[ STATUS KOMPONEN ]");

  display.setTextSize(1);

  // Baris 1: OLED SSD1306
  display.setCursor(0, 14);
  display.print("OLED  : ");
  if (state.oledOk) {
    display.print("OK (0x3C)");
  } else {
    display.print("MISSING");
  }

  // Baris 2: Ultrasonic Exit (Lane Keluar)
  display.setCursor(0, 24);
  display.print("US-EXIT: ");
  if (state.usExitOk) {
    display.print("OK (");
    display.print((int)state.jarakKeluar);
    display.print("cm)");
  } else {
    display.print("MISSING");
  }

  // Baris 3: Alur Masuk (Keypad)
  display.setCursor(0, 34);
  display.print("MASUK : KEYPAD TIKET");

  // Baris 4: Servo Gate SG90
  display.setCursor(0, 44);
  display.print("SERVO : ");
  if (state.servoOk) {
    display.print("OK (");
    display.print(state.gateAngle);
    display.print(" deg)");
  } else {
    display.print("MISSING");
  }

  // Baris 5: Keypad Membran 4x4
  display.setCursor(0, 54);
  display.print("KEYPAD: OK  [C:Menu]");
}
