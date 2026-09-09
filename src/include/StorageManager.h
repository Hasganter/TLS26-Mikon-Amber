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

private:
  Preferences prefs;
};
