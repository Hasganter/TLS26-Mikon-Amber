#include "include/StorageManager.h"

StorageManager::StorageManager() {}

int StorageManager::inisialisasi(int kapasitasMaksimal) {
  prefs.begin("parking", false);
  int slot = prefs.getInt("slots", kapasitasMaksimal);

  // Validasi nilai jika pertama kali dijalankan atau korup
  if (slot < 0 || slot > kapasitasMaksimal) {
    slot = kapasitasMaksimal;
    prefs.putInt("slots", slot);
  }
  return slot;
}

void StorageManager::simpanSlot(int nilaiSlot) {
  prefs.putInt("slots", nilaiSlot);
}
