#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <Arduino.h>

// ═════════════════════════════════════════════════════════════
// STATUS SISTEM (FINITE STATE MACHINE)
// ═════════════════════════════════════════════════════════════

enum StatusSistem {
  STATUS_STANDBY,          // Standby menampilkan info slot besar & waktu
  STATUS_COUNTDOWN_5S,      // Hitung mundur 5 detik buka gate (failsafe / cancel)
  STATUS_GATE_TERBUKA,      // Gerbang terbuka & safety hold mobil melintas
  STATUS_LANE_KELUAR,       // Lane keluar mendeteksi kendaraan & tambah slot
  STATUS_DEBUG_MENU,        // Menu utama konfigurasi debug
  STATUS_DEBUG_WAKTU,       // Sub-menu pengaturan jam (HHMMSS)
  STATUS_DEBUG_TANGGAL,     // Sub-menu pengaturan tanggal (DDMMYYYY)
  STATUS_DEBUG_SLOT,        // Sub-menu pengaturan slot parkir terpadu (max 999)
  STATUS_DEBUG_WIFI_SCAN,   // Sub-menu daftar pemindaian jaringan WiFi
  STATUS_DEBUG_WIFI_PASS,   // Sub-menu input password WiFi via T9
  STATUS_DEBUG_WIFI_MANUAL  // Sub-menu input manual SSID WiFi via T9
};

// ═════════════════════════════════════════════════════════════
// MODE INPUT TEKS KEYPAD (MULTI-TAP T9)
// ═════════════════════════════════════════════════════════════

enum InputMode {
  MODE_ABC,  // Huruf kapital (A-Z)
  MODE_abc,  // Huruf kecil (a-z)
  MODE_123   // Angka langsung (0-9)
};

// ═════════════════════════════════════════════════════════════
// STATUS KONEKSI WIFI
// ═════════════════════════════════════════════════════════════

enum WiFiConnectionStatus {
  WIFI_STATUS_DISCONNECTED,
  WIFI_STATUS_SCANNING,
  WIFI_STATUS_CONNECTING,
  WIFI_STATUS_CONNECTED,
  WIFI_STATUS_FAILED
};

// ═════════════════════════════════════════════════════════════
// STRUCT BUNDLE DATA UNTUK RENDERING UI
// ═════════════════════════════════════════════════════════════

struct UIState {
  StatusSistem status;
  int slotTersedia;
  int kapasitasMaksimal;
  bool layarStandbyModeA;
  float jarakKiri;
  float jarakKanan;

  // Pewaktu
  unsigned long sisaCountdown5s;
  bool mobilSedangDiBawahGate;
  bool mobilPernahTerdeteksiDiGate;
  unsigned long sisaTutupAmanMs;
  unsigned long sisaTimeoutGateMs;

  // Waktu terformat
  const char* bufWaktu;
  const char* bufTanggal;
  const char* bufHari;

  // Input Debug standar
  String bufferInputDebug;
  String pesanFeedbackDebug;
  bool fokusMaksimal;
  String bufferTersedia;
  String bufferMaksimal;

  // Context WiFi
  int wifiSelectedIndex;
  int wifiScrollOffset;
  bool isScanningWiFi;
  int wifiTotalNetworks;
  String wifiTargetSSID;
  WiFiConnectionStatus wifiStatus;
  String wifiAssignedIP;

  // Helper Input T9
  String t9DisplayText;
  const char* t9ModeStr;
};

#endif // SYSTEM_TYPES_H
