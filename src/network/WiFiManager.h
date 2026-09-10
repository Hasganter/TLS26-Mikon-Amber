#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "../config/SystemTypes.h"
#include "../config/Config.h"
#include "../system/StorageManager.h"

class WiFiManager {
public:
  WiFiManager();

  // Inisialisasi WiFi station mode
  void inisialisasi();

  // Mulai pemindaian asinkron (non-blocking)
  void mulaiPindai();

  // Update status pemindaian dan koneksi secara berkala
  void update();

  // Mulai proses koneksi ke SSID tertentu
  void hubungkan(const String &ssid, const String &password);

  // Putuskan koneksi WiFi
  void putuskan();

  // Cek apakah ada kredensial tersimpan di NVS dan konek otomatis
  void autoConnectJikaTersimpan(StorageManager &storage);

  // Status & Getters
  bool isSedangMemindai() const;
  int getJumlahJaringan() const;
  String getSSID(int index) const;
  int32_t getRSSI(int index) const;
  bool isEncrypted(int index) const;

  WiFiConnectionStatus getStatusKoneksi() const;
  String getIP() const;
  String getTargetSSID() const;
  bool isTerhubung() const;

private:
  bool sedangMemindai;
  int jumlahJaringan;
  WiFiConnectionStatus statusKoneksi;
  unsigned long waktuMulaiKoneksi;
  String targetSSID;
  String assignedIP;
};

#endif // WIFI_MANAGER_H
