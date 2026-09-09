#include "include/StorageManager.h"

StorageManager::StorageManager() {}

void StorageManager::inisialisasi(int defaultMaxSlots, int &outSlotTersedia, int &outKapasitasMaksimal) {
  prefs.begin("parking", false);

  int maxSlots = prefs.getInt("max_slots", defaultMaxSlots);
  if (maxSlots < 1) {
    maxSlots = defaultMaxSlots;
    prefs.putInt("max_slots", maxSlots);
  }

  int slot = prefs.getInt("slots", maxSlots);
  if (slot < 0 || slot > maxSlots) {
    slot = maxSlots;
    prefs.putInt("slots", slot);
  }

  outKapasitasMaksimal = maxSlots;
  outSlotTersedia = slot;
}

void StorageManager::simpanSlot(int nilaiSlot) {
  prefs.putInt("slots", nilaiSlot);
}

void StorageManager::simpanKapasitasMaksimal(int nilaiMax) {
  prefs.putInt("max_slots", nilaiMax);
}
