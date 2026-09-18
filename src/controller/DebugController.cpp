#include "DebugController.h"

DebugController::DebugController(
  ParkingController &p,
  StorageManager &s,
  WiFiManager &w
) : parking(p),
    storage(s),
    wifi(w),
    waktuAktivitasTerakhir(0),
    waktuFeedback(0),
    pesanFeedback(""),
    muteMissingNotifier(false),
    bufferInput(""),
    fokusMaksimal(false),
    bufferTersedia(""),
    bufferMaksimal(""),
    wifiSelectedIndex(0),
    wifiScrollOffset(0),
    wifiTargetSSID("") {}

void DebugController::resetActivity(unsigned long sekarang) {
  waktuAktivitasTerakhir = sekarang;
}

bool DebugController::isMuteMissingNotifier() const {
  return muteMissingNotifier;
}

void DebugController::toggleMuteMissingNotifier() {
  muteMissingNotifier = !muteMissingNotifier;
}

void DebugController::handleLoop(StatusSistem &status, char tombol, unsigned long sekarang) {
  // Timeout 30 detik kembali ke standby
  if (sekarang - waktuAktivitasTerakhir >= TIMEOUT_DEBUG_MS) {
    status = STATUS_STANDBY;
    return;
  }

  // Update T9 multi-tap commit timeout
  inputHelper.update();

  switch (status) {
    // STATUS_DEBUG_MENU: Menu Utama Konfigurasi
    case STATUS_DEBUG_MENU: {
      if (tombol != '\0') {
        resetActivity(sekarang);
        bufferInput = "";
        pesanFeedback = "";

        if (tombol == '1') {
          status = STATUS_DEBUG_WAKTU;
        } else if (tombol == '2') {
          status = STATUS_DEBUG_TANGGAL;
        } else if (tombol == '3') {
          status = STATUS_DEBUG_SLOT;
          fokusMaksimal = false;
          bufferTersedia = "";
          bufferMaksimal = "";
        } else if (tombol == '4') {
          status = STATUS_DEBUG_WIFI_SCAN;
          wifiSelectedIndex = 0;
          wifiScrollOffset = 0;
          wifi.mulaiPindai();
        } else if (tombol == '5') {
          status = STATUS_DEBUG_KOMPONEN;
        } else if (tombol == '6') {
          muteMissingNotifier = !muteMissingNotifier;
        } else if (tombol == 'C') {
          status = STATUS_STANDBY;
        }
      }
      break;
    }

    // STATUS_DEBUG_WAKTU: Pengaturan Waktu (HHMMSS)
    case STATUS_DEBUG_WAKTU: {
      if (pesanFeedback.length() > 0 && sekarang - waktuFeedback >= 2000) {
        pesanFeedback = "";
        status = STATUS_DEBUG_MENU;
        break;
      }

      if (tombol != '\0' && pesanFeedback.length() == 0) {
        resetActivity(sekarang);
        if (tombol >= '0' && tombol <= '9') {
          if (bufferInput.length() < 6) bufferInput += tombol;
        } else if (tombol == '*') {
          if (bufferInput.length() > 0) bufferInput.remove(bufferInput.length() - 1);
        } else if (tombol == 'C') {
          status = STATUS_DEBUG_MENU;
        } else if (tombol == '#') {
          if (TimeManager::setWaktuDariString(bufferInput)) {
            pesanFeedback = "Waktu Disimpan!";
          } else {
            pesanFeedback = "Waktu Invalid!";
            bufferInput = "";
          }
          waktuFeedback = sekarang;
        }
      }
      break;
    }

    // STATUS_DEBUG_TANGGAL: Pengaturan Tanggal (DDMMYYYY)
    case STATUS_DEBUG_TANGGAL: {
      if (pesanFeedback.length() > 0 && sekarang - waktuFeedback >= 2000) {
        pesanFeedback = "";
        status = STATUS_DEBUG_MENU;
        break;
      }

      if (tombol != '\0' && pesanFeedback.length() == 0) {
        resetActivity(sekarang);
        if (tombol >= '0' && tombol <= '9') {
          if (bufferInput.length() < 8) bufferInput += tombol;
        } else if (tombol == '*') {
          if (bufferInput.length() > 0) bufferInput.remove(bufferInput.length() - 1);
        } else if (tombol == 'C') {
          status = STATUS_DEBUG_MENU;
        } else if (tombol == '#') {
          if (TimeManager::setTanggalDariString(bufferInput)) {
            pesanFeedback = "Tanggal Disimpan!";
          } else {
            pesanFeedback = "Tanggal Invalid!";
            bufferInput = "";
          }
          waktuFeedback = sekarang;
        }
      }
      break;
    }

    // STATUS_DEBUG_SLOT: Pengaturan Slot Parkir Terpadu (Max 999)
    case STATUS_DEBUG_SLOT: {
      if (pesanFeedback.length() > 0 && sekarang - waktuFeedback >= 2000) {
        pesanFeedback = "";
        status = STATUS_DEBUG_MENU;
        break;
      }

      if (tombol != '\0' && pesanFeedback.length() == 0) {
        resetActivity(sekarang);

        if (tombol == 'A') {
          // Beralih fokus aktif antara Tersedia dan Maksimal
          fokusMaksimal = !fokusMaksimal;
        } else if (tombol >= '0' && tombol <= '9') {
          // Mendukung hingga 3 digit (max 999)
          if (fokusMaksimal) {
            if (bufferMaksimal.length() < 3) bufferMaksimal += tombol;
          } else {
            if (bufferTersedia.length() < 3) bufferTersedia += tombol;
          }
        } else if (tombol == '*') {
          if (fokusMaksimal) {
            if (bufferMaksimal.length() > 0) bufferMaksimal.remove(bufferMaksimal.length() - 1);
          } else {
            if (bufferTersedia.length() > 0) bufferTersedia.remove(bufferTersedia.length() - 1);
          }
        } else if (tombol == 'C') {
          status = STATUS_DEBUG_MENU;
        } else if (tombol == '#') {
          int newMax = (bufferMaksimal.length() > 0) ? bufferMaksimal.toInt() : parking.getKapasitasMaksimal();
          int newSlot = (bufferTersedia.length() > 0) ? bufferTersedia.toInt() : parking.getSlotTersedia();

          if (newMax >= 1 && newMax <= BATAS_MAX_SLOT && newSlot >= 0 && newSlot <= newMax) {
            parking.setKapasitasMaksimal(newMax);
            parking.setSlotTersedia(newSlot);
            storage.simpanKapasitasMaksimal(newMax);
            storage.simpanSlot(newSlot);
            pesanFeedback = "Slot Disimpan!";
          } else {
            pesanFeedback = "Nilai Invalid!";
            bufferTersedia = "";
            bufferMaksimal = "";
          }
          waktuFeedback = sekarang;
        }
      }
      break;
    }

    // STATUS_DEBUG_WIFI_SCAN: Pemindaian & Pemilihan Jaringan WiFi
    case STATUS_DEBUG_WIFI_SCAN: {
      int totalItems = wifi.getJumlahJaringan() + 1; // Termasuk [+ Input Manual]

      if (tombol != '\0') {
        resetActivity(sekarang);

        if (tombol == 'C') {
          status = STATUS_DEBUG_MENU;
        } else if (tombol == '*') {
          // Pindai Ulang
          wifiSelectedIndex = 0;
          wifiScrollOffset = 0;
          wifi.mulaiPindai();
        } else if (!wifi.isSedangMemindai()) {
          if (tombol == 'A') {
            // Scroll Atas
            if (wifiSelectedIndex > 0) {
              wifiSelectedIndex--;
              if (wifiSelectedIndex < wifiScrollOffset) {
                wifiScrollOffset = wifiSelectedIndex;
              }
            }
          } else if (tombol == 'B') {
            // Scroll Bawah
            if (wifiSelectedIndex < totalItems - 1) {
              wifiSelectedIndex++;
              if (wifiSelectedIndex >= wifiScrollOffset + 3) {
                wifiScrollOffset = wifiSelectedIndex - 2;
              }
            }
          } else if (tombol == '#') {
            // Pilih Jaringan
            if (wifiSelectedIndex == 0) {
              // Mode Input Manual SSID
              wifiTargetSSID = "";
              inputHelper.reset("");
              inputHelper.setMode(MODE_abc);
              status = STATUS_DEBUG_WIFI_MANUAL;
            } else {
              int netIdx = wifiSelectedIndex - 1;
              if (netIdx >= 0 && netIdx < wifi.getJumlahJaringan()) {
                wifiTargetSSID = wifi.getSSID(netIdx);

                if (!wifi.isEncrypted(netIdx)) {
                  // Jaringan terbuka (tanpa password): langsung hubungkan!
                  wifi.hubungkan(wifiTargetSSID, "");
                  status = STATUS_DEBUG_WIFI_PASS; // Menampilkan status koneksi
                } else {
                  // Jaringan ber-password
                  inputHelper.reset("");
                  inputHelper.setMode(MODE_abc);
                  status = STATUS_DEBUG_WIFI_PASS;
                }
              }
            }
          }
        }
      }
      break;
    }

    // STATUS_DEBUG_WIFI_MANUAL: Input Manual SSID via T9
    case STATUS_DEBUG_WIFI_MANUAL: {
      if (tombol != '\0') {
        resetActivity(sekarang);

        if (tombol == 'C') {
          status = STATUS_DEBUG_WIFI_SCAN;
        } else if (tombol == '#') {
          wifiTargetSSID = inputHelper.getCommittedText();
          if (wifiTargetSSID.length() > 0) {
            inputHelper.reset("");
            inputHelper.setMode(MODE_abc);
            status = STATUS_DEBUG_WIFI_PASS;
          }
        } else {
          inputHelper.handleKey(tombol);
        }
      }
      break;
    }

    // STATUS_DEBUG_WIFI_PASS: Input Password / Status Koneksi
    case STATUS_DEBUG_WIFI_PASS: {
      WiFiConnectionStatus stat = wifi.getStatusKoneksi();

      if (stat == WIFI_STATUS_CONNECTING) {
        // Sedang menghubungkan
        if (tombol == 'C') {
          wifi.putuskan();
          status = STATUS_DEBUG_WIFI_SCAN;
        }
      } else if (stat == WIFI_STATUS_CONNECTED) {
        // Berhasil konek
        if (tombol == '#') {
          status = STATUS_DEBUG_MENU;
        }
      } else if (stat == WIFI_STATUS_FAILED) {
        // Gagal konek
        if (tombol == '*') {
          // Coba lagi dengan password yang sama
          wifi.hubungkan(wifiTargetSSID, inputHelper.getCommittedText());
        } else if (tombol == 'C') {
          status = STATUS_DEBUG_WIFI_SCAN;
        }
      } else {
        // Form pengetikan password via T9
        if (tombol != '\0') {
          resetActivity(sekarang);

          if (tombol == 'C') {
            status = STATUS_DEBUG_WIFI_SCAN;
          } else if (tombol == '#') {
            String pass = inputHelper.getCommittedText();
            wifi.hubungkan(wifiTargetSSID, pass);
            // Simpan otomatis ke NVS
            storage.simpanWiFi(wifiTargetSSID, pass);
          } else {
            inputHelper.handleKey(tombol);
          }
        }
      }
      break;
    }

    // STATUS_DEBUG_KOMPONEN: Monitoring status live seluruh komponen
    case STATUS_DEBUG_KOMPONEN: {
      if (tombol != '\0') {
        resetActivity(sekarang);
        if (tombol == 'C') {
          status = STATUS_DEBUG_MENU;
        }
      }
      break;
    }

    default:
      break;
  }
}

void DebugController::populateUIState(UIState &state) {
  state.bufferInputDebug = bufferInput;
  state.pesanFeedbackDebug = pesanFeedback;
  state.fokusMaksimal = fokusMaksimal;
  state.bufferTersedia = bufferTersedia;
  state.bufferMaksimal = bufferMaksimal;

  state.wifiSelectedIndex = wifiSelectedIndex;
  state.wifiScrollOffset = wifiScrollOffset;
  state.isScanningWiFi = wifi.isSedangMemindai();
  state.wifiTotalNetworks = wifi.getJumlahJaringan();
  state.wifiTargetSSID = wifiTargetSSID;
  state.wifiStatus = wifi.getStatusKoneksi();
  state.wifiAssignedIP = wifi.getIP();

  state.t9DisplayText = inputHelper.getDisplayText(true);
  state.t9ModeStr = inputHelper.getModeStr();

  state.muteMissingNotifier = muteMissingNotifier;
}
