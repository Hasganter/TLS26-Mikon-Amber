/*
 * Proyek: TLS26 Mikon Amber — Sistem Counter & Portal Parkir Otomatis Dual Lane
 * Platform: ESP32 DevKit C V4
 *
 * Struktur Berkas Modular (src/):
 *   - src/Config.h            : Definisi pinout, konstanta waktu, dan enum FSM
 *   - src/TimeManager.h/.cpp   : Software RTC (POSIX time, format, set waktu/tanggal)
 *   - src/StorageManager.h/.cpp: Non-Volatile Flash Storage (Preferences/NVS)
 *   - src/GateServo.h/.cpp     : Kontrol Motor Servo SG90 Portal Masuk
 *   - src/UltrasonicManager.h/.cpp : Polling non-blocking 2x sensor HC-SR04
 *   - src/KeypadManager.h/.cpp : Pembacaan matriks 4x4 keypad membran
 *   - src/DisplayManager.h/.cpp: Rendering antarmuka OLED 128x64 SSD1306
 */

#include "src/include/Config.h"
#include "src/include/TimeManager.h"
#include "src/include/StorageManager.h"
#include "src/include/GateServo.h"
#include "src/include/UltrasonicManager.h"
#include "src/include/KeypadManager.h"
#include "src/include/DisplayManager.h"

// ═════════════════════════════════════════════════════════════
// INSTANSIASI MODUL MANAJER
// ═════════════════════════════════════════════════════════════

GateServo gate;
UltrasonicManager ultrasonic;
KeypadManager keypadMgr;
StorageManager storage;
DisplayManager displayMgr;

// ═════════════════════════════════════════════════════════════
// VARIABEL STATUS SISTEM & PEWAKTU
// ═════════════════════════════════════════════════════════════

StatusSistem statusSaatIni = STATUS_STANDBY;
int kapasitasMaksimal = DEFAULT_KAPASITAS_MAKSIMAL;
int slotTersedia = DEFAULT_KAPASITAS_MAKSIMAL;

// Pewaktu siklus standby (30 detik per layar)
unsigned long waktuSiklusStandby = 0;
bool layarStandbyModeA = true;

// Pewaktu alur masuk & gate
unsigned long waktuMulaiCountdown5s = 0;
unsigned long waktuMulaiGateBuka = 0;
unsigned long waktuMobilSelesaiLewat = 0;
bool mobilSedangDiBawahGate = false;
bool mobilPernahTerdeteksiDiGate = false;
bool gateDibukaLewatSensor = false;

// Pelacakan tombol pintasan Debug Mode (Bebas + D + D + D)
int hitunganTombolD = 0;

// Variabel kerja Debug Mode
String bufferInputDebug = "";
String pesanFeedbackDebug = "";
unsigned long waktuFeedbackDebug = 0;
unsigned long waktuAktivitasDebugTerakhir = 0;
bool fokusMaksimal = false;
String bufferInputTersedia = "";
String bufferInputMaksimal = "";

// ═════════════════════════════════════════════════════════════
// SETUP
// ═════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  Serial.println("\n[SISTEM PARKIR] Memulai inisialisasi modul...");

  // Inisialisasi Layar OLED
  if (!displayMgr.inisialisasi()) {
    Serial.println("[DISPLAY] Gagal menginisialisasi OLED SSD1306!");
  }

  // Inisialisasi Sensor Ultrasonik Ganda
  ultrasonic.inisialisasi();

  // Inisialisasi Motor Servo Gate
  gate.inisialisasi(PIN_SERVO_GATE);

  // Inisialisasi NVS Flash Preferences
  storage.inisialisasi(DEFAULT_KAPASITAS_MAKSIMAL, slotTersedia, kapasitasMaksimal);
  Serial.printf("[NVS] Slot dimuat: %d / %d\n", slotTersedia, kapasitasMaksimal);

  // Inisialisasi Software RTC
  TimeManager::inisialisasiWaktu();

  waktuSiklusStandby = millis();
  Serial.println("[SISTEM PARKIR] Seluruh modul siap beroperasi.");
}

// ═════════════════════════════════════════════════════════════
// LOOP UTAMA
// ═════════════════════════════════════════════════════════════

void loop() {
  unsigned long sekarang = millis();

  // 1. Perbarui pembacaan sensor ultrasonik non-blocking
  ultrasonic.perbarui();

  // 2. Baca tombol keypad
  char tombol = keypadMgr.bacaTombol();

  // 3. Logika State Machine
  switch (statusSaatIni) {

    // ─────────────────────────────────────────────────────────
    // STATUS_STANDBY: Alur Default 1 & Deteksi Masuk/Keluar
    // ─────────────────────────────────────────────────────────
    case STATUS_STANDBY: {
      // Pergantian layar A/B setiap 30 detik
      if (sekarang - waktuSiklusStandby >= PERIODE_STANDBY_SCREEN) {
        waktuSiklusStandby = sekarang;
        layarStandbyModeA = !layarStandbyModeA;
      }

      // Deteksi Mobil Keluar di Lane Kanan
      if (ultrasonic.isMobilTerdeteksiKanan()) {
        statusSaatIni = STATUS_LANE_KELUAR;
        Serial.println("[LANE KELUAR] Mobil terdeteksi keluar.");
        break;
      }

      // Deteksi Tombol Keypad Ditekan
      if (tombol != NO_KEY) {
        Serial.printf("[KEYPAD] Tombol ditekan: %c\n", tombol);

        // Jika parkir penuh: Tolak pembukaan gate
        if (slotTersedia <= 0) {
          Serial.println("[MASUK] Ditolak: Parkir Penuh!");
        } else {
          statusSaatIni = STATUS_COUNTDOWN_5S;
          waktuMulaiCountdown5s = sekarang;
          hitunganTombolD = 0;

          // Fast Bypass: Jika mobil sudah di depan sensor kiri, langsung buka!
          if (ultrasonic.isMobilTerdeteksiKiri()) {
            Serial.println("[MASUK] Fast Bypass: Sensor kiri aktif, membuka gate.");
            statusSaatIni = STATUS_GATE_TERBUKA;
            waktuMulaiGateBuka = sekarang;
            gateDibukaLewatSensor = true;
            mobilSedangDiBawahGate = true;
            mobilPernahTerdeteksiDiGate = true;
            gate.buka();
          }
        }
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_COUNTDOWN_5S: Jendela Pembatalan & Pintasan Debug
    // ─────────────────────────────────────────────────────────
    case STATUS_COUNTDOWN_5S: {
      if (tombol != NO_KEY) {
        if (tombol == 'C') {
          Serial.println("[COUNTDOWN 5s] Dibatalkan oleh tombol C.");
          statusSaatIni = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
          break;
        } else if (tombol == 'D') {
          hitunganTombolD++;
          Serial.printf("[DEBUG SHORTCUT] D ke-%d\n", hitunganTombolD);
          if (hitunganTombolD >= 3) {
            Serial.println("[DEBUG MODE] Akses diterima!");
            statusSaatIni = STATUS_DEBUG_MENU;
            waktuAktivitasDebugTerakhir = sekarang;
            bufferInputDebug = "";
            pesanFeedbackDebug = "";
            break;
          }
        } else {
          hitunganTombolD = 0;
        }
      }

      // Fast Bypass jika mobil mendekat selama hitung mundur
      if (ultrasonic.isMobilTerdeteksiKiri()) {
        Serial.println("[COUNTDOWN 5s] Mobil terdeteksi! Fast bypass membuka gate.");
        statusSaatIni = STATUS_GATE_TERBUKA;
        waktuMulaiGateBuka = sekarang;
        gateDibukaLewatSensor = true;
        mobilSedangDiBawahGate = true;
        mobilPernahTerdeteksiDiGate = true;
        gate.buka();
        break;
      }

      // Timeout 5 detik habis -> Buka Gate (Failsafe)
      if (sekarang - waktuMulaiCountdown5s >= TIMEOUT_COUNTDOWN_5S) {
        Serial.println("[COUNTDOWN 5s] Failsafe: Membuka gate.");
        statusSaatIni = STATUS_GATE_TERBUKA;
        waktuMulaiGateBuka = sekarang;
        gateDibukaLewatSensor = false;
        mobilSedangDiBawahGate = ultrasonic.isMobilTerdeteksiKiri();
        mobilPernahTerdeteksiDiGate = mobilSedangDiBawahGate;
        gate.buka();
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_GATE_TERBUKA: Mekanisme Portal Masuk & Safety Hold
    // ─────────────────────────────────────────────────────────
    case STATUS_GATE_TERBUKA: {
      bool deteksiSekarang = ultrasonic.isMobilTerdeteksiKiri();

      if (deteksiSekarang) {
        mobilSedangDiBawahGate = true;
        mobilPernahTerdeteksiDiGate = true;
      } else {
        if (mobilSedangDiBawahGate) {
          mobilSedangDiBawahGate = false;
          waktuMobilSelesaiLewat = sekarang;
          Serial.println("[GATE] Mobil selesai lewat. Memulai hitung mundur 5s tutup.");
        }
      }

      // Penutupan 5 Detik setelah mobil lewat
      if (mobilPernahTerdeteksiDiGate && !mobilSedangDiBawahGate) {
        if (sekarang - waktuMobilSelesaiLewat >= TIMEOUT_TUTUP_AMAN_5S) {
          Serial.println("[GATE] Penutupan aman selesai. Tutup gate & kurangi slot.");
          gate.tutup();
          if (slotTersedia > 0) {
            slotTersedia--;
            storage.simpanSlot(slotTersedia);
          }
          statusSaatIni = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
          break;
        }
      }

      // Safety Timeout 30 Detik
      if (sekarang - waktuMulaiGateBuka >= TIMEOUT_GATE_OPEN_MAX) {
        Serial.println("[GATE] Timeout 30 detik habis. Tutup gate.");
        gate.tutup();

        if (!gateDibukaLewatSensor) {
          Serial.println("[GATE] Dibuka via tombol failsafe -> Kurangi slot.");
          if (slotTersedia > 0) {
            slotTersedia--;
            storage.simpanSlot(slotTersedia);
          }
        }
        statusSaatIni = STATUS_STANDBY;
        waktuSiklusStandby = sekarang;
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_LANE_KELUAR: Alur Keluar & Tambah Slot
    // ─────────────────────────────────────────────────────────
    case STATUS_LANE_KELUAR: {
      if (!ultrasonic.isMobilTerdeteksiKanan()) {
        Serial.println("[LANE KELUAR] Mobil selesai keluar. Tambah slot.");
        if (slotTersedia < kapasitasMaksimal) {
          slotTersedia++;
          storage.simpanSlot(slotTersedia);
        }
        delay(800);
        statusSaatIni = STATUS_STANDBY;
        waktuSiklusStandby = sekarang;
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_MENU: Menu Utama Konfigurasi
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_MENU: {
      if (sekarang - waktuAktivitasDebugTerakhir >= TIMEOUT_DEBUG_MS) {
        statusSaatIni = STATUS_STANDBY;
        waktuSiklusStandby = sekarang;
        break;
      }

      if (tombol != NO_KEY) {
        waktuAktivitasDebugTerakhir = sekarang;
        bufferInputDebug = "";
        pesanFeedbackDebug = "";

        if (tombol == '1') statusSaatIni = STATUS_DEBUG_WAKTU;
        else if (tombol == '2') statusSaatIni = STATUS_DEBUG_TANGGAL;
        else if (tombol == '3') {
          statusSaatIni = STATUS_DEBUG_SLOT;
          fokusMaksimal = false;
          bufferInputTersedia = "";
          bufferInputMaksimal = "";
        }
        else if (tombol == 'C') {
          statusSaatIni = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
        }
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_WAKTU: Sub-menu Edit Waktu (HHMM)
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_WAKTU: {
      if (sekarang - waktuAktivitasDebugTerakhir >= TIMEOUT_DEBUG_MS) {
        statusSaatIni = STATUS_STANDBY;
        break;
      }

      if (pesanFeedbackDebug.length() > 0 && sekarang - waktuFeedbackDebug >= 2000) {
        pesanFeedbackDebug = "";
        statusSaatIni = STATUS_DEBUG_MENU;
        break;
      }

      if (tombol != NO_KEY && pesanFeedbackDebug.length() == 0) {
        waktuAktivitasDebugTerakhir = sekarang;
        if (tombol >= '0' && tombol <= '9') {
          if (bufferInputDebug.length() < 4) bufferInputDebug += tombol;
        } else if (tombol == '*') {
          if (bufferInputDebug.length() > 0) bufferInputDebug.remove(bufferInputDebug.length() - 1);
        } else if (tombol == 'C') {
          statusSaatIni = STATUS_DEBUG_MENU;
        } else if (tombol == '#') {
          if (TimeManager::setWaktuDariString(bufferInputDebug)) {
            pesanFeedbackDebug = "Waktu Disimpan!";
          } else {
            pesanFeedbackDebug = "Waktu Invalid!";
            bufferInputDebug = "";
          }
          waktuFeedbackDebug = sekarang;
        }
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_TANGGAL: Sub-menu Edit Tanggal (DDMMYYYY)
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_TANGGAL: {
      if (sekarang - waktuAktivitasDebugTerakhir >= TIMEOUT_DEBUG_MS) {
        statusSaatIni = STATUS_STANDBY;
        break;
      }

      if (pesanFeedbackDebug.length() > 0 && sekarang - waktuFeedbackDebug >= 2000) {
        pesanFeedbackDebug = "";
        statusSaatIni = STATUS_DEBUG_MENU;
        break;
      }

      if (tombol != NO_KEY && pesanFeedbackDebug.length() == 0) {
        waktuAktivitasDebugTerakhir = sekarang;
        if (tombol >= '0' && tombol <= '9') {
          if (bufferInputDebug.length() < 8) bufferInputDebug += tombol;
        } else if (tombol == '*') {
          if (bufferInputDebug.length() > 0) bufferInputDebug.remove(bufferInputDebug.length() - 1);
        } else if (tombol == 'C') {
          statusSaatIni = STATUS_DEBUG_MENU;
        } else if (tombol == '#') {
          if (TimeManager::setTanggalDariString(bufferInputDebug)) {
            pesanFeedbackDebug = "Tanggal Disimpan!";
          } else {
            pesanFeedbackDebug = "Tanggal Invalid!";
            bufferInputDebug = "";
          }
          waktuFeedbackDebug = sekarang;
        }
      }
      break;
    }

    // ─────────────────────────────────────────────────────────
    // STATUS_DEBUG_SLOT: Sub-menu Edit Slot Parkir Terpadu
    // ─────────────────────────────────────────────────────────
    case STATUS_DEBUG_SLOT: {
      if (sekarang - waktuAktivitasDebugTerakhir >= TIMEOUT_DEBUG_MS) {
        statusSaatIni = STATUS_STANDBY;
        break;
      }

      if (pesanFeedbackDebug.length() > 0 && sekarang - waktuFeedbackDebug >= 2000) {
        pesanFeedbackDebug = "";
        statusSaatIni = STATUS_DEBUG_MENU;
        break;
      }

      if (tombol != NO_KEY && pesanFeedbackDebug.length() == 0) {
        waktuAktivitasDebugTerakhir = sekarang;

        if (tombol == 'A') {
          // Beralih field aktif antara Tersedia dan Maksimal
          fokusMaksimal = !fokusMaksimal;
        } else if (tombol >= '0' && tombol <= '9') {
          if (fokusMaksimal) {
            if (bufferInputMaksimal.length() < 2) bufferInputMaksimal += tombol;
          } else {
            if (bufferInputTersedia.length() < 2) bufferInputTersedia += tombol;
          }
        } else if (tombol == '*') {
          if (fokusMaksimal) {
            if (bufferInputMaksimal.length() > 0) bufferInputMaksimal.remove(bufferInputMaksimal.length() - 1);
          } else {
            if (bufferInputTersedia.length() > 0) bufferInputTersedia.remove(bufferInputTersedia.length() - 1);
          }
        } else if (tombol == 'C') {
          statusSaatIni = STATUS_DEBUG_MENU;
        } else if (tombol == '#') {
          int newMax = (bufferInputMaksimal.length() > 0) ? bufferInputMaksimal.toInt() : kapasitasMaksimal;
          int newSlot = (bufferInputTersedia.length() > 0) ? bufferInputTersedia.toInt() : slotTersedia;

          if (newMax >= 1 && newMax <= 99 && newSlot >= 0 && newSlot <= newMax) {
            kapasitasMaksimal = newMax;
            slotTersedia = newSlot;
            storage.simpanKapasitasMaksimal(kapasitasMaksimal);
            storage.simpanSlot(slotTersedia);
            pesanFeedbackDebug = "Slot Disimpan!";
          } else {
            pesanFeedbackDebug = "Nilai Invalid!";
            bufferInputTersedia = "";
            bufferInputMaksimal = "";
          }
          waktuFeedbackDebug = sekarang;
        }
      }
      break;
    }
  }

  // 4. Hitung sisa waktu untuk visualisasi
  unsigned long sisaCountdown5s = 0;
  if (statusSaatIni == STATUS_COUNTDOWN_5S) {
    unsigned long durasi = sekarang - waktuMulaiCountdown5s;
    sisaCountdown5s = (durasi < TIMEOUT_COUNTDOWN_5S) ? (TIMEOUT_COUNTDOWN_5S - durasi) : 0;
  }

  unsigned long sisaTutupAmanMs = 0;
  if (statusSaatIni == STATUS_GATE_TERBUKA && mobilPernahTerdeteksiDiGate && !mobilSedangDiBawahGate) {
    unsigned long durasi = sekarang - waktuMobilSelesaiLewat;
    sisaTutupAmanMs = (durasi < TIMEOUT_TUTUP_AMAN_5S) ? (TIMEOUT_TUTUP_AMAN_5S - durasi) : 0;
  }

  unsigned long sisaTimeoutGateMs = 0;
  if (statusSaatIni == STATUS_GATE_TERBUKA) {
    unsigned long durasi = sekarang - waktuMulaiGateBuka;
    sisaTimeoutGateMs = (durasi < TIMEOUT_GATE_OPEN_MAX) ? (TIMEOUT_GATE_OPEN_MAX - durasi) : 0;
  }

  char bufWaktu[16], bufTanggal[16], bufHari[16];
  TimeManager::dapatkanWaktuFormat(bufWaktu, bufTanggal, bufHari);

  // 5. Render antarmuka visual OLED
  displayMgr.render(
    statusSaatIni,
    slotTersedia,
    kapasitasMaksimal,
    layarStandbyModeA,
    ultrasonic.getJarakKiri(),
    sisaCountdown5s,
    mobilSedangDiBawahGate,
    mobilPernahTerdeteksiDiGate,
    sisaTutupAmanMs,
    sisaTimeoutGateMs,
    bufWaktu,
    bufTanggal,
    bufHari,
    bufferInputDebug,
    pesanFeedbackDebug,
    fokusMaksimal,
    bufferInputTersedia,
    bufferInputMaksimal
  );
}
