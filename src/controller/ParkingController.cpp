#include "ParkingController.h"

ParkingController::ParkingController(GateServo &g, UltrasonicManager &u, StorageManager &s)
  : gate(g),
    ultrasonic(u),
    storage(s),
    slotTersedia(DEFAULT_KAPASITAS_MAKSIMAL),
    kapasitasMaksimal(DEFAULT_KAPASITAS_MAKSIMAL),
    waktuSiklusStandby(0),
    layarStandbyModeA(true),
    waktuMulaiCountdown5s(0),
    waktuMulaiGateBuka(0),
    waktuMobilSelesaiLewat(0),
    mobilSedangDiBawahGate(false),
    mobilPernahTerdeteksiDiGate(false),
    gateDibukaLewatSensor(false),
    hitunganTombolD(0),
    waktuTombolDTerakhir(0),
    gateTerkunci(false),
    waktuMobilKeluarSelesai(0),
    mobilKeluarPending(false) {}

void ParkingController::inisialisasi() {
  storage.inisialisasi(DEFAULT_KAPASITAS_MAKSIMAL, slotTersedia, kapasitasMaksimal);
  waktuSiklusStandby = millis();
}

void ParkingController::bukaManual() {
  if (!gateTerkunci) {
    gate.buka();
    Serial.println("[REMOTE] Portal dibuka manual via Web.");
  } else {
    Serial.println("[REMOTE] Gagal: Portal sedang dikunci darurat!");
  }
}

void ParkingController::tutupManual() {
  gate.tutup();
  Serial.println("[REMOTE] Portal ditutup manual via Web.");
}

void ParkingController::toggleKunciDarurat() {
  gateTerkunci = !gateTerkunci;
  if (gateTerkunci) {
    gate.tutup();
    Serial.println("[REMOTE] Portal DIKUNCI DARURAT!");
  } else {
    Serial.println("[REMOTE] Kunci darurat portal dilepas.");
  }
}

bool ParkingController::isGateTerkunci() const {
  return gateTerkunci;
}

bool ParkingController::isGateTerbuka() const {
  return gate.isTerbuka();
}

int ParkingController::getSudutGate() const {
  return gate.isTerbuka() ? SERVO_BUKA : SERVO_TUTUP;
}

void ParkingController::handleLoop(StatusSistem &status, char tombol, unsigned long sekarang) {
  switch (status) {
    // STATUS_STANDBY: Alur Default 1 & Deteksi Masuk/Keluar
    case STATUS_STANDBY: {
      // Siklus 30 detik berganti antara Mode A dan Mode B
      if (sekarang - waktuSiklusStandby >= PERIODE_STANDBY_SCREEN) {
        waktuSiklusStandby = sekarang;
        layarStandbyModeA = !layarStandbyModeA;
      }

      // Deteksi mobil keluar di Lane Kanan
      if (ultrasonic.isMobilTerdeteksiKanan()) {
        status = STATUS_LANE_KELUAR;
        mobilKeluarPending = false;
        Serial.println("[LANE KELUAR] Mobil terdeteksi keluar.");
        break;
      }

      // Deteksi tombol keypad
      if (tombol != '\0') {
        Serial.printf("[KEYPAD] Tombol ditekan: %c\r\n", tombol);

        // Prioritas Utama: Pintasan Masuk Debug Mode via 'D' (3x tekan)
        // Tombol 'D' SELALU diproses untuk debug, tidak pernah diblokir oleh slot penuh atau gate terkunci!
        if (tombol == 'D') {
          if (sekarang - waktuTombolDTerakhir > 3000) {
            hitunganTombolD = 0;
          }
          waktuTombolDTerakhir = sekarang;
          hitunganTombolD++;
          Serial.printf("[DEBUG SHORTCUT] D ke-%d (Standby)\r\n", hitunganTombolD);
          if (hitunganTombolD >= 3) {
            hitunganTombolD = 0;
            status = STATUS_DEBUG_MENU;
            Serial.println("[DEBUG MODE] Akses diterima via Standby!");
            break;
          }

          // Jika D ditekan 1x (atau belum 3x), tetap masuki hitung mundur 5 detik buka gate!
          if (gateTerkunci) {
            Serial.println("[MASUK] Ditolak: Gerbang Dikunci Darurat!");
            break;
          } else if (slotTersedia <= 0) {
            Serial.println("[MASUK] Ditolak: Parkir Penuh!");
            break;
          } else {
            status = STATUS_COUNTDOWN_5S;
            waktuMulaiCountdown5s = sekarang;
            Serial.println("[COUNTDOWN 5s] Dimulai via tombol D (1x)");
            break;
          }
        } else {
          hitunganTombolD = 0;
        }

        // Jika gate terkunci darurat: tolak
        if (gateTerkunci) {
          Serial.println("[MASUK] Ditolak: Gerbang Dikunci Darurat!");
        } else if (slotTersedia <= 0) {
          // Jika parkir penuh: tolak pembukaan portal
          Serial.println("[MASUK] Ditolak: Parkir Penuh!");
        } else {
          status = STATUS_COUNTDOWN_5S;
          waktuMulaiCountdown5s = sekarang;

          // Fast Bypass: jika mobil sudah di depan sensor kiri, langsung buka
          if (ultrasonic.isMobilTerdeteksiKiri()) {
            Serial.println("[MASUK] Fast Bypass: Sensor kiri aktif, membuka gate.");
            status = STATUS_GATE_TERBUKA;
            waktuMulaiGateBuka = sekarang;
            gateDibukaLewatSensor = true;
            mobilSedangDiBawahGate = true;
            mobilPernahTerdeteksiDiGate = true;
            gate.buka();
          }
        }
      }
      break;
    }

    // STATUS_COUNTDOWN_5S: Jendela Pembatalan & Pintasan Debug
    case STATUS_COUNTDOWN_5S: {
      if (tombol != '\0') {
        if (tombol == 'C') {
          Serial.println("[COUNTDOWN 5s] Dibatalkan oleh tombol C.");
          status = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
          hitunganTombolD = 0;
          break;
        } else if (tombol == 'D') {
          if (sekarang - waktuTombolDTerakhir > 3000) {
            hitunganTombolD = 0;
          }
          waktuTombolDTerakhir = sekarang;
          hitunganTombolD++;
          Serial.printf("[DEBUG SHORTCUT] D ke-%d (Countdown)\r\n", hitunganTombolD);
          if (hitunganTombolD >= 3) {
            hitunganTombolD = 0;
            Serial.println("[DEBUG MODE] Akses diterima via Countdown!");
            status = STATUS_DEBUG_MENU;
            break;
          }
        } else {
          hitunganTombolD = 0;
        }
      }

      // Fast Bypass jika mobil mendekat selama countdown (hanya jika bukan sedang mengetik shortcut debug)
      if (ultrasonic.isMobilTerdeteksiKiri() && !gateTerkunci && hitunganTombolD == 0) {
        Serial.println("[COUNTDOWN 5s] Mobil terdeteksi! Fast bypass membuka gate.");
        status = STATUS_GATE_TERBUKA;
        waktuMulaiGateBuka = sekarang;
        gateDibukaLewatSensor = true;
        mobilSedangDiBawahGate = true;
        mobilPernahTerdeteksiDiGate = true;
        gate.buka();
        break;
      }

      // Timeout 5 detik habis -> Buka Gate (Failsafe)
      if (sekarang - waktuMulaiCountdown5s >= TIMEOUT_COUNTDOWN_5S) {
        if (!gateTerkunci) {
          Serial.println("[COUNTDOWN 5s] Failsafe: Membuka gate.");
          status = STATUS_GATE_TERBUKA;
          waktuMulaiGateBuka = sekarang;
          gateDibukaLewatSensor = false;
          mobilSedangDiBawahGate = ultrasonic.isMobilTerdeteksiKiri();
          mobilPernahTerdeteksiDiGate = mobilSedangDiBawahGate;
          gate.buka();
        } else {
          status = STATUS_STANDBY;
        }
      }
      break;
    }

    // STATUS_GATE_TERBUKA: Mekanisme Portal Masuk & Safety Hold
    case STATUS_GATE_TERBUKA: {
      bool deteksiSekarang = ultrasonic.isMobilTerdeteksiKiri();

      if (deteksiSekarang) {
        mobilSedangDiBawahGate = true;
        mobilPernahTerdeteksiDiGate = true;
      } else {
        if (mobilSedangDiBawahGate) {
          mobilSedangDiBawahGate = false;
          waktuMobilSelesaiLewat = sekarang;
          Serial.println("[GATE] Mobil selesai lewat. Memulai hitung mundur 5s tutup.");
        }
      }

      // Opsi Tutup Sekarang setelah 5 detik portal dibuka (jika mobil tidak sedang di bawah gate)
      if (sekarang - waktuMulaiGateBuka >= 5000) {
        if (!mobilSedangDiBawahGate && (tombol == 'C' || tombol == '#')) {
          Serial.println("[GATE] Portal ditutup manual via tombol C setelah 5 detik terbuka.");
          gate.tutup();
          if (slotTersedia > 0) {
            slotTersedia--;
            storage.simpanSlot(slotTersedia);
          }
          status = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
          break;
        }
      }

      // Penutupan 5 Detik setelah mobil lewat
      if (mobilPernahTerdeteksiDiGate && !mobilSedangDiBawahGate) {
        if (sekarang - waktuMobilSelesaiLewat >= TIMEOUT_TUTUP_AMAN_5S) {
          Serial.println("[GATE] Penutupan aman selesai. Tutup gate & kurangi slot.");
          gate.tutup();
          if (slotTersedia > 0) {
            slotTersedia--;
            storage.simpanSlot(slotTersedia);
          }
          status = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
          break;
        }
      }

      // Safety Timeout 30 Detik
      if (sekarang - waktuMulaiGateBuka >= TIMEOUT_GATE_OPEN_MAX) {
        Serial.println("[GATE] Timeout 30 detik habis. Tutup gate.");
        gate.tutup();

        if (!gateDibukaLewatSensor) {
          Serial.println("[GATE] Dibuka via tombol failsafe -> Kurangi slot.");
          if (slotTersedia > 0) {
            slotTersedia--;
            storage.simpanSlot(slotTersedia);
          }
        }
        status = STATUS_STANDBY;
        waktuSiklusStandby = sekarang;
      }
      break;
    }

    // STATUS_LANE_KELUAR: Alur Keluar & Tambah Slot
    case STATUS_LANE_KELUAR: {
      if (!mobilKeluarPending) {
        if (!ultrasonic.isMobilTerdeteksiKanan()) {
          Serial.println("[LANE KELUAR] Mobil selesai keluar. Tambah slot.");
          if (slotTersedia < kapasitasMaksimal) {
            slotTersedia++;
            storage.simpanSlot(slotTersedia);
          }
          mobilKeluarPending = true;
          waktuMobilKeluarSelesai = sekarang;
        }
      } else {
        // Tahan tampilan pesan sampai 800ms secara non-blocking
        if (sekarang - waktuMobilKeluarSelesai >= 800) {
          mobilKeluarPending = false;
          status = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
        }
      }
      break;
    }

    default:
      break;
  }
}

int ParkingController::getSlotTersedia() const {
  return slotTersedia;
}

int ParkingController::getKapasitasMaksimal() const {
  return kapasitasMaksimal;
}

void ParkingController::setSlotTersedia(int val) {
  slotTersedia = val;
}

void ParkingController::setKapasitasMaksimal(int val) {
  kapasitasMaksimal = val;
}

bool ParkingController::isLayarStandbyModeA() const {
  return layarStandbyModeA;
}

unsigned long ParkingController::getSisaCountdown5s() const {
  unsigned long sekarang = millis();
  unsigned long durasi = sekarang - waktuMulaiCountdown5s;
  return (durasi < TIMEOUT_COUNTDOWN_5S) ? (TIMEOUT_COUNTDOWN_5S - durasi) : 0;
}

bool ParkingController::isMobilSedangDiBawahGate() const {
  return mobilSedangDiBawahGate;
}

bool ParkingController::isMobilPernahTerdeteksiDiGate() const {
  return mobilPernahTerdeteksiDiGate;
}

unsigned long ParkingController::getSisaTutupAmanMs() const {
  unsigned long sekarang = millis();
  unsigned long durasi = sekarang - waktuMobilSelesaiLewat;
  return (durasi < TIMEOUT_TUTUP_AMAN_5S) ? (TIMEOUT_TUTUP_AMAN_5S - durasi) : 0;
}

unsigned long ParkingController::getSisaTimeoutGateMs() const {
  unsigned long sekarang = millis();
  unsigned long durasi = sekarang - waktuMulaiGateBuka;
  return (durasi < TIMEOUT_GATE_OPEN_MAX) ? (TIMEOUT_GATE_OPEN_MAX - durasi) : 0;
}
