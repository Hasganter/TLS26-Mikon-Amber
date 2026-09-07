#include "include/DisplayManager.h"

DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

bool DisplayManager::inisialisasi() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    return false;
  }
  display.clearDisplay();
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
  const String &pesanFeedbackDebug
) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  switch (status) {
    case STATUS_STANDBY: {
      // Header: Info Slot dan Jam
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("SLOT: ");
      display.print(slotTersedia);
      display.print("/");
      display.print(kapasitasMaksimal);

      display.setCursor(80, 0);
      display.print(bufWaktu);
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      if (slotTersedia <= 0) {
        display.setTextSize(2);
        display.setCursor(4, 20);
        display.print("PARKIR");
        display.setCursor(4, 38);
        display.print("PENUH!");
        display.setTextSize(1);
        display.setCursor(0, 56);
        display.print("Gerbang Masuk Terkunci");
      } else {
        if (jarakKiri < AMBANG_DETEKSI_CM) {
          // Ada kendaraan di depan sensor masuk
          display.setTextSize(1);
          display.setCursor(0, 16);
          display.print("Kendaraan Terdeteksi!");
          display.setTextSize(2);
          display.setCursor(0, 28);
          display.print("TEKAN");
          display.setCursor(0, 46);
          display.print("TOMBOL TIKET");
        } else if (layarStandbyModeA) {
          // Standby Tampilan A: Ucapan Selamat Datang
          display.setTextSize(2);
          display.setCursor(4, 20);
          display.print("SELAMAT");
          display.setCursor(4, 38);
          display.print("DATANG");
          display.setTextSize(1);
          display.setCursor(0, 56);
          display.print("Tekan Tombol Utk Masuk");
        } else {
          // Standby Tampilan B: Jam dan Tanggal Real-Time
          display.setTextSize(1);
          display.setCursor(0, 16);
          display.print(bufHari);
          display.print(", ");
          display.print(bufTanggal);

          display.setTextSize(2);
          display.setCursor(14, 32);
          display.print(bufWaktu);

          display.setTextSize(1);
          display.setCursor(0, 56);
          display.print("Slot Tersedia: ");
          display.print(slotTersedia);
        }
      }
      break;
    }

    case STATUS_COUNTDOWN_5S: {
      int sisaDetik = (sisaCountdown5s / 1000) + 1;
      if (sisaDetik > 5) sisaDetik = 5;
      if (sisaDetik < 1) sisaDetik = 1;

      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ PROSES MASUK ]");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      display.setTextSize(1);
      display.setCursor(0, 16);
      display.print("Membuka gate dalam:");

      display.setTextSize(3);
      display.setCursor(54, 28);
      display.print(sisaDetik);

      display.setTextSize(1);
      display.setCursor(0, 56);
      display.print("Tekan C : Batalkan");
      break;
    }

    case STATUS_GATE_TERBUKA: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ PORTAL TERBUKA ]");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      if (mobilSedangDiBawahGate) {
        display.setTextSize(1);
        display.setCursor(0, 16);
        display.print("Status: Mobil Melintas");
        display.setTextSize(2);
        display.setCursor(10, 32);
        display.print("SILAKAN");
        display.setCursor(26, 48);
        display.print("MASUK");
      } else if (mobilPernahTerdeteksiDiGate) {
        int sisaDetik = (sisaTutupAmanMs / 1000) + 1;
        if (sisaDetik < 1) sisaDetik = 1;

        display.setTextSize(1);
        display.setCursor(0, 16);
        display.print("Mobil Selesai Lewat!");
        display.setCursor(0, 30);
        display.print("Tutup Gate dalam:");
        display.setTextSize(2);
        display.setCursor(54, 44);
        display.print(sisaDetik);
        display.print("s");
      } else {
        int sisaDetik = (sisaTimeoutGateMs / 1000) + 1;
        if (sisaDetik < 1) sisaDetik = 1;

        display.setTextSize(1);
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

    case STATUS_LANE_KELUAR: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ LANE KELUAR ]");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      display.setTextSize(2);
      display.setCursor(4, 18);
      display.print("SAMPAI");
      display.setCursor(4, 34);
      display.print("JUMPA LAGI");

      display.setTextSize(1);
      display.setCursor(0, 54);
      display.print("Hati-hati di jalan!");
      break;
    }

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
      display.print("3: Slot   (Manual)");
      display.setCursor(0, 50);
      display.print("C: Keluar ke Standby");
      break;
    }

    case STATUS_DEBUG_WAKTU: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ SET WAKTU (24h) ]");
      display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

      if (pesanFeedbackDebug.length() > 0) {
        display.setTextSize(1);
        display.setCursor(0, 24);
        display.print(pesanFeedbackDebug);
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
        display.print("*:Del  #:Simpan C:Batal");
      }
      break;
    }

    case STATUS_DEBUG_TANGGAL: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ SET TANGGAL ]");
      display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

      if (pesanFeedbackDebug.length() > 0) {
        display.setTextSize(1);
        display.setCursor(0, 24);
        display.print(pesanFeedbackDebug);
      } else {
        display.setCursor(0, 14);
        display.print("Tgl skrg: ");
        display.print(bufTanggal);

        display.setCursor(0, 26);
        display.print("Format  : DDMMYYYY");

        display.setTextSize(1);
        display.setCursor(8, 40);
        display.print("Input: [");
        display.print(bufferInputDebug);
        for (int i = bufferInputDebug.length(); i < 8; i++) display.print("_");
        display.print("]");

        display.setCursor(0, 56);
        display.print("*:Del  #:Simpan C:Batal");
      }
      break;
    }

    case STATUS_DEBUG_SLOT: {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("[ ATUR SLOT MANUAL ]");
      display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

      if (pesanFeedbackDebug.length() > 0) {
        display.setTextSize(1);
        display.setCursor(0, 24);
        display.print(pesanFeedbackDebug);
      } else {
        display.setCursor(0, 13);
        display.print("Slot Skrg: ");
        display.print(slotTersedia);
        display.print(" / ");
        display.print(kapasitasMaksimal);

        display.setCursor(0, 24);
        display.print("A: +1 Slot | B: -1 Slot");

        display.setCursor(0, 36);
        display.print("Atau Ketik: 0-");
        display.print(kapasitasMaksimal);
        display.print(" [");
        display.print(bufferInputDebug);
        display.print("]");

        display.setCursor(0, 54);
        display.print("#:Simpan  C:Kembali");
      }
      break;
    }
  }

  display.display();
}
