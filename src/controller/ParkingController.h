#pragma once
#include <Arduino.h>
#include "../config/SystemTypes.h"
#include "../config/Config.h"
#include "../hardware/GateServo.h"
#include "../hardware/UltrasonicManager.h"
#include "../system/StorageManager.h"

class ParkingController {
public:
  ParkingController(GateServo &gate, UltrasonicManager &ultrasonic, StorageManager &storage);

  void inisialisasi();
  void handleLoop(StatusSistem &status, char tombol, unsigned long sekarang);

  // Kontrol Jarak Jauh (Web Manual Control)
  void bukaManual();
  void tutupManual();
  void toggleKunciDarurat();
  bool isGateTerkunci() const;
  bool isGateTerbuka() const;
  int getSudutGate() const;

  // Getters & Setters Kuota
  int getSlotTersedia() const;
  int getKapasitasMaksimal() const;
  void setSlotTersedia(int val);
  void setKapasitasMaksimal(int val);

  bool isLayarStandbyModeA() const;
  unsigned long getSisaCountdown5s() const;
  bool isMobilSedangDiBawahGate() const;
  bool isMobilPernahTerdeteksiDiGate() const;
  unsigned long getSisaTutupAmanMs() const;
  unsigned long getSisaTimeoutGateMs() const;

private:
  GateServo &gate;
  UltrasonicManager &ultrasonic;
  StorageManager &storage;

  int slotTersedia;
  int kapasitasMaksimal;

  unsigned long waktuSiklusStandby;
  bool layarStandbyModeA;

  unsigned long waktuMulaiCountdown5s;
  unsigned long waktuMulaiGateBuka;
  unsigned long waktuMobilSelesaiLewat;
  bool mobilSedangDiBawahGate;
  bool mobilPernahTerdeteksiDiGate;
  bool gateDibukaLewatSensor;

  int hitunganTombolD;
  unsigned long waktuTombolDTerakhir;
  bool gateTerkunci;

  unsigned long waktuMobilKeluarSelesai;
  bool mobilKeluarPending;
};

