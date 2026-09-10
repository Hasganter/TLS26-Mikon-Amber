#pragma once
#include <Arduino.h>
#include <Preferences.h>

class StorageManager {
public:
  StorageManager();

  // Inisialisasi NVS dan muat nilai slot serta kapasitas maksimal
  void inisialisasi(int defaultMaxSlots, int &outSlotTersedia, int &outKapasitasMaksimal);

  // Simpan nilai slot baru ke memori NVS
  void simpanSlot(int nilaiSlot);

  // Simpan kapasitas maksimal baru ke memori NVS
  void simpanKapasitasMaksimal(int nilaiMax);

  // Simpan kredensial WiFi
  void simpanWiFi(const String &ssid, const String &password);

  // Muat kredensial WiFi, mengembalikan true jika ada tersimpan
  bool muatWiFi(String &outSsid, String &outPassword);

  // Hapus kredensial WiFi
  void hapusWiFi();

private:
  Preferences prefs;
};

