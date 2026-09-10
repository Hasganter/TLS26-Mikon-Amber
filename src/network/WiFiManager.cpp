#include "WiFiManager.h"

WiFiManager::WiFiManager()
  : sedangMemindai(false),
    jumlahJaringan(0),
    statusKoneksi(WIFI_STATUS_DISCONNECTED),
    waktuMulaiKoneksi(0),
    targetSSID(""),
    assignedIP(""),
    softAPIP("192.168.4.1") {}

void WiFiManager::inisialisasi() {
  // Aktifkan mode ganda (Station + Hotspot SoftAP)
  WiFi.mode(WIFI_AP_STA);

  // Nyalakan SoftAP cadangan
  WiFi.softAP(AP_FALLBACK_SSID, AP_FALLBACK_PASS);
  softAPIP = WiFi.softAPIP().toString();
  Serial.printf("[WIFI] Hotspot SoftAP Aktif: SSID: '%s' | IP: http://%s\r\n", AP_FALLBACK_SSID, softAPIP.c_str());

  WiFi.disconnect();
}

void WiFiManager::mulaiPindai() {
  sedangMemindai = true;
  jumlahJaringan = 0;
  // Non-blocking asynchronous scan
  WiFi.scanNetworks(true, false);
}

void WiFiManager::update() {
  // Update status pemindaian
  if (sedangMemindai) {
    int16_t status = WiFi.scanComplete();
    if (status >= 0) {
      sedangMemindai = false;
      jumlahJaringan = status;
    } else if (status == WIFI_SCAN_FAILED) {
      sedangMemindai = false;
      jumlahJaringan = 0;
    }
  }

  // Update status proses koneksi
  if (statusKoneksi == WIFI_STATUS_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      statusKoneksi = WIFI_STATUS_CONNECTED;
      assignedIP = WiFi.localIP().toString();
      Serial.printf("[WIFI] Berhasil terhubung ke '%s'! IP STA: http://%s\r\n", targetSSID.c_str(), assignedIP.c_str());
    } else if (millis() - waktuMulaiKoneksi >= WIFI_CONNECT_TIMEOUT_MS) {
      statusKoneksi = WIFI_STATUS_FAILED;
      WiFi.disconnect();
      Serial.println("[WIFI] Batas waktu koneksi habis (Timeout).");
    }
  } else if (statusKoneksi == WIFI_STATUS_CONNECTED) {
    if (WiFi.status() != WL_CONNECTED) {
      statusKoneksi = WIFI_STATUS_DISCONNECTED;
      assignedIP = "";
      Serial.println("[WIFI] Terputus dari router jaringan.");
    }
  }
}

void WiFiManager::hubungkan(const String &ssid, const String &password) {
  targetSSID = ssid;
  statusKoneksi = WIFI_STATUS_CONNECTING;
  waktuMulaiKoneksi = millis();
  assignedIP = "";

  Serial.printf("[WIFI] Menghubungkan ke SSID: %s...\r\n", ssid.c_str());
  if (password.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
  } else {
    WiFi.begin(ssid.c_str());
  }
}

void WiFiManager::putuskan() {
  WiFi.disconnect();
  statusKoneksi = WIFI_STATUS_DISCONNECTED;
  assignedIP = "";
  targetSSID = "";
}

void WiFiManager::autoConnectJikaTersimpan(StorageManager &storage) {
  String savedSSID, savedPass;
  if (storage.muatWiFi(savedSSID, savedPass)) {
    Serial.printf("[WIFI] Kredensial tersimpan ditemukan: %s. Menghubungkan di background...\r\n", savedSSID.c_str());
    hubungkan(savedSSID, savedPass);
  } else {
    Serial.println("[WIFI] Tidak ada kredensial WiFi tersimpan.");
  }
}

bool WiFiManager::isSedangMemindai() const {
  return sedangMemindai;
}

int WiFiManager::getJumlahJaringan() const {
  return jumlahJaringan;
}

String WiFiManager::getSSID(int index) const {
  if (index < 0 || index >= jumlahJaringan) return "";
  return WiFi.SSID(index);
}

int32_t WiFiManager::getRSSI(int index) const {
  if (index < 0 || index >= jumlahJaringan) return -100;
  return WiFi.RSSI(index);
}

bool WiFiManager::isEncrypted(int index) const {
  if (index < 0 || index >= jumlahJaringan) return false;
  return (WiFi.encryptionType(index) != WIFI_AUTH_OPEN);
}

WiFiConnectionStatus WiFiManager::getStatusKoneksi() const {
  return statusKoneksi;
}

String WiFiManager::getIP() const {
  return assignedIP;
}

String WiFiManager::getSoftAPIP() const {
  return softAPIP;
}

int32_t WiFiManager::getActiveRSSI() const {
  if (statusKoneksi == WIFI_STATUS_CONNECTED) {
    return WiFi.RSSI();
  }
  return -100;
}

String WiFiManager::getTargetSSID() const {
  return targetSSID;
}

bool WiFiManager::isTerhubung() const {
  return (statusKoneksi == WIFI_STATUS_CONNECTED);
}
