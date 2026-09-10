#include "StorageManager.h"

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
  if (prefs.getInt("slots", -1) != nilaiSlot) {
    prefs.putInt("slots", nilaiSlot);
  }
}

void StorageManager::simpanKapasitasMaksimal(int nilaiMax) {
  if (prefs.getInt("max_slots", -1) != nilaiMax) {
    prefs.putInt("max_slots", nilaiMax);
  }
}

void StorageManager::simpanWiFi(const String &ssid, const String &password) {
  prefs.putString("wifi_ssid", ssid);
  prefs.putString("wifi_pass", password);
}

bool StorageManager::muatWiFi(String &outSsid, String &outPassword) {
  outSsid = prefs.getString("wifi_ssid", "");
  outPassword = prefs.getString("wifi_pass", "");
  return (outSsid.length() > 0);
}

void StorageManager::hapusWiFi() {
  prefs.remove("wifi_ssid");
  prefs.remove("wifi_pass");
}
