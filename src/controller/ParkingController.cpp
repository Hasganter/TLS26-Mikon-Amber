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
    mobilKeluarPending(false),
    waktuGateDitutup(0) {}

void ParkingController::inisialisasi() {
  storage.inisialisasi(DEFAULT_KAPASITAS_MAKSIMAL, slotTersedia, kapasitasMaksimal);
  waktuSiklusStandby = millis();
}

void ParkingController::bukaManual() {
  if (!gateTerkunci) {
    gate.buka();
    ultrasonic.tunda(BUFFER_SERVO_GERAK_MS);
    Serial.println("[REMOTE] Portal dibuka manual via Web.");
  } else {
    Serial.println("[REMOTE] Gagal: Portal sedang dikunci darurat!");
  }
}

void ParkingController::tutupManual() {
  gate.tutup();
  ultrasonic.tunda(DELAY_SENSOR_SETELAH_GATE_MS);
  waktuGateDitutup = millis();
  Serial.println("[REMOTE] Portal ditutup manual via Web.");
}

void ParkingController::toggleKunciDarurat() {
  gateTerkunci = !gateTerkunci;
  if (gateTerkunci) {
    gate.tutup();
    ultrasonic.tunda(DELAY_SENSOR_SETELAH_GATE_MS);
    waktuGateDitutup = millis();
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
    // STATUS_STANDBY: Alur Default & Deteksi Masuk/Keluar
    case STATUS_STANDBY: {
      // Siklus 30 detik berganti antara Mode A dan Mode B
      if (sekarang - waktuSiklusStandby >= PERIODE_STANDBY_SCREEN) {
        waktuSiklusStandby = sekarang;
        layarStandbyModeA = !layarStandbyModeA;
      }

      // Deteksi mobil keluar via sensor ultrasonik tunggal (Lane Keluar)
      // Diblokir penuh saat servo sedang/baru saja diperintahkan bergerak dengan buffer waktu
      if (!gate.isSedangBergerak() && !ultrasonic.isDitunda() && (sekarang - waktuGateDitutup >= BUFFER_SERVO_GERAK_MS)) {
        if (ultrasonic.isMobilTerdeteksiKeluar()) {
          status = STATUS_LANE_KELUAR;
          mobilKeluarPending = false;
          Serial.println("[LANE KELUAR] Mobil terdeteksi keluar.");
          break;
        }
      }

      // Deteksi tombol keypad untuk alur masuk
      if (tombol != '\0') {
        Serial.printf("[KEYPAD] Tombol ditekan: %c\r\n", tombol);

        // Jika tombol 'D' ditekan: antisipasi 3x 'D' untuk debug shortcut dengan cooldown 5s
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

          if (gateTerkunci) {
            Serial.println("[MASUK] Ditolak: Gerbang Dikunci Darurat!");
            break;
          } else if (slotTersedia <= 0) {
            Serial.println("[MASUK] Ditolak: Parkir Penuh!");
            break;
          } else {
            status = STATUS_COUNTDOWN_5S;
            waktuMulaiCountdown5s = sekarang;
            Serial.println("[COUNTDOWN 5s] Dimulai via tombol D (antisipasi 3x D)");
            break;
          }
        } else {
          // Tombol selain 'D': Langsung membuka gate!
          hitunganTombolD = 0;

          if (gateTerkunci) {
            Serial.println("[MASUK] Ditolak: Gerbang Dikunci Darurat!");
          } else if (slotTersedia <= 0) {
            Serial.println("[MASUK] Ditolak: Parkir Penuh!");
          } else {
            Serial.println("[MASUK] Tombol ditekan -> Langsung membuka gate!");
            status = STATUS_GATE_TERBUKA;
            waktuMulaiGateBuka = sekarang;
            gateDibukaLewatSensor = false;
            mobilSedangDiBawahGate = false;
            mobilPernahTerdeteksiDiGate = false;
            gate.buka();
            ultrasonic.tunda(BUFFER_SERVO_GERAK_MS);
          }
        }
      }
      break;
    }

    // STATUS_COUNTDOWN_5S: Cooldown antisipasi 3x D
    case STATUS_COUNTDOWN_5S: {
      if (tombol != '\0') {
        if (tombol == 'D') {
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
          // Jika menekan tombol selain D setelah D pertama: langsung buka gate!
          hitunganTombolD = 0;
          if (!gateTerkunci && slotTersedia > 0) {
            Serial.println("[COUNTDOWN 5s] Tombol selain D ditekan -> Langsung membuka gate!");
            status = STATUS_GATE_TERBUKA;
            waktuMulaiGateBuka = sekarang;
            gateDibukaLewatSensor = false;
            mobilSedangDiBawahGate = false;
            mobilPernahTerdeteksiDiGate = false;
            gate.buka();
            ultrasonic.tunda(BUFFER_SERVO_GERAK_MS);
            break;
          } else {
            status = STATUS_STANDBY;
            waktuSiklusStandby = sekarang;
            break;
          }
        }
      }

      // Timeout 5 detik habis -> Buka Gate
      if (sekarang - waktuMulaiCountdown5s >= TIMEOUT_COUNTDOWN_5S) {
        if (!gateTerkunci && slotTersedia > 0) {
          Serial.println("[COUNTDOWN 5s] Timeout 5 detik selesai -> Membuka gate.");
          status = STATUS_GATE_TERBUKA;
          waktuMulaiGateBuka = sekarang;
          gateDibukaLewatSensor = false;
          mobilSedangDiBawahGate = false;
          mobilPernahTerdeteksiDiGate = false;
          gate.buka();
          ultrasonic.tunda(BUFFER_SERVO_GERAK_MS);
        } else {
          status = STATUS_STANDBY;
          waktuSiklusStandby = sekarang;
        }
      }
      break;
    }

    // STATUS_GATE_TERBUKA: Mekanisme Portal Masuk & Penutupan
    case STATUS_GATE_TERBUKA: {
      // Opsi Tutup Sekarang setelah 5 detik portal dibuka via tombol C atau #
      if (sekarang - waktuMulaiGateBuka >= 5000) {
        if (tombol == 'C' || tombol == '#') {
          Serial.println("[GATE] Portal ditutup manual via tombol C/# setelah 5 detik.");
          gate.tutup();
          ultrasonic.tunda(DELAY_SENSOR_SETELAH_GATE_MS);
          waktuGateDitutup = sekarang;
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
        Serial.println("[GATE] Timeout 30 detik habis. Tutup gate & kurangi slot.");
        gate.tutup();
        ultrasonic.tunda(DELAY_SENSOR_SETELAH_GATE_MS);
        waktuGateDitutup = sekarang;
        if (slotTersedia > 0) {
          slotTersedia--;
          storage.simpanSlot(slotTersedia);
        }
        status = STATUS_STANDBY;
        waktuSiklusStandby = sekarang;
      }
      break;
    }

    // STATUS_LANE_KELUAR: Alur Keluar & Tambah Slot
    case STATUS_LANE_KELUAR: {
      if (!mobilKeluarPending) {
        if (!ultrasonic.isMobilTerdeteksiKeluar()) {
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
