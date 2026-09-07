#pragma once
#include <Arduino.h>
#include <Preferences.h>

class StorageManager {
public:
  StorageManager();

  // Inisialisasi NVS dan muat nilai slot tersimpan
  int inisialisasi(int kapasitasMaksimal);

  // Simpan nilai slot baru ke memori NVS
  void simpanSlot(int nilaiSlot);

private:
  Preferences prefs;
};
