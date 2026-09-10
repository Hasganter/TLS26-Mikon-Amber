/*
 * Proyek: TLS26 Mikon Amber — Sistem Counter & Portal Parkir Otomatis Dual Lane
 * Platform: ESP32 DevKit C V4
 *
 * Arsitektur Berkas Modular (src/):
 *   - src/config/     : Config.h, SystemTypes.h
 *   - src/hardware/   : GateServo, UltrasonicManager, KeypadManager
 *   - src/system/     : StorageManager (NVS), TimeManager (Software RTC)
 *   - src/input/      : KeypadInputHelper (Multi-Tap T9 Text Engine)
 *   - src/network/    : WiFiManager (Async Scan & Connect)
 *   - src/ui/         : DisplayManager, UIHelper, UIViewParking, UIViewDebug
 *   - src/controller/ : ParkingController, DebugController
 */

#include "src/config/Config.h"
#include "src/config/SystemTypes.h"
#include "src/hardware/GateServo.h"
#include "src/hardware/UltrasonicManager.h"
#include "src/hardware/KeypadManager.h"
#include "src/system/StorageManager.h"
#include "src/system/TimeManager.h"
#include "src/network/WiFiManager.h"
#include "src/ui/DisplayManager.h"
#include "src/controller/ParkingController.h"
#include "src/controller/DebugController.h"

// ═════════════════════════════════════════════════════════════
// INSTANSIASI MODUL & KONTROLER
// ═════════════════════════════════════════════════════════════

GateServo gate;
UltrasonicManager ultrasonic;
KeypadManager keypadMgr;
StorageManager storage;
WiFiManager wifi;
DisplayManager displayMgr;

ParkingController parking(gate, ultrasonic, storage);
DebugController debugCtrl(parking, storage, wifi);

StatusSistem statusSaatIni = STATUS_STANDBY;

// ═════════════════════════════════════════════════════════════
// SETUP SISTEM
// ═════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  Serial.println("\n[SISTEM PARKIR] Memulai inisialisasi modul modular...");

  // Inisialisasi Layar OLED SSD1306
  if (!displayMgr.inisialisasi()) {
    Serial.println("[DISPLAY] Gagal menginisialisasi OLED SSD1306!");
  }

  // Inisialisasi Hardware
  ultrasonic.inisialisasi();
  gate.inisialisasi(PIN_SERVO_GATE);

  // Inisialisasi Software RTC & Storage
  TimeManager::inisialisasiWaktu();
  parking.inisialisasi();

  // Inisialisasi Jaringan WiFi & Auto-Connect
  wifi.inisialisasi();
  wifi.autoConnectJikaTersimpan(storage);

  Serial.println("[SISTEM PARKIR] Seluruh modul siap beroperasi.");
}

// ═════════════════════════════════════════════════════════════
// LOOP UTAMA
// ═════════════════════════════════════════════════════════════

void loop() {
  unsigned long sekarang = millis();

  // 1. Polling Sensor Ultrasonik & WiFi secara non-blocking
  ultrasonic.perbarui();
  wifi.update();

  // 2. Baca Tombol Keypad
  char tombol = keypadMgr.bacaTombol();

  // 3. Delegasi FSM ke Kontroler Sesuai Domain
  if (statusSaatIni <= STATUS_LANE_KELUAR) {
    parking.handleLoop(statusSaatIni, tombol, sekarang);
  } else {
    debugCtrl.handleLoop(statusSaatIni, tombol, sekarang);
  }

  // 4. Siapkan Data Bundle UIState
  char bufWaktu[16], bufTanggal[16], bufHari[16];
  TimeManager::dapatkanWaktuFormat(bufWaktu, bufTanggal, bufHari);

  UIState state;
  state.status = statusSaatIni;
  state.slotTersedia = parking.getSlotTersedia();
  state.kapasitasMaksimal = parking.getKapasitasMaksimal();
  state.layarStandbyModeA = parking.isLayarStandbyModeA();
  state.jarakKiri = ultrasonic.getJarakKiri();
  state.jarakKanan = ultrasonic.getJarakKanan();

  state.sisaCountdown5s = parking.getSisaCountdown5s();
  state.mobilSedangDiBawahGate = parking.isMobilSedangDiBawahGate();
  state.mobilPernahTerdeteksiDiGate = parking.isMobilPernahTerdeteksiDiGate();
  state.sisaTutupAmanMs = parking.getSisaTutupAmanMs();
  state.sisaTimeoutGateMs = parking.getSisaTimeoutGateMs();

  state.bufWaktu = bufWaktu;
  state.bufTanggal = bufTanggal;
  state.bufHari = bufHari;

  // Isi data debug ke bundle UIState
  debugCtrl.populateUIState(state);

  // Tandai dirty jika ada penekanan tombol
  if (tombol != '\0' && tombol != NO_KEY) {
    displayMgr.markDirty();
  }

  // 5. Render Layar OLED dengan Frame Throttling
  displayMgr.render(state, wifi);
}
