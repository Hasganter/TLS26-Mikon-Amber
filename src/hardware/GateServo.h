#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "../config/Config.h"

class GateServo {
public:
  GateServo();

  // Inisialisasi pin PWM dan atur portal ke posisi tertutup
  void inisialisasi(uint8_t pinServo);

  // Buka portal (90 derajat)
  void buka();

  // Tutup portal (0 derajat)
  void tutup();

  // Status apakah portal sedang terbuka
  bool isTerbuka() const;

  // Status kesehatan & deteksi fisik koneksi servo
  bool isOk();
  int getSudut() const;

private:
  bool deteksiHardware();

  Servo servo;
  uint8_t pinPwm;
  int posisiSudut;
  bool terhubung;
  unsigned long waktuCekTerakhir;
};

