#ifndef GATE_SERVO_H
#define GATE_SERVO_H

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

private:
  Servo servo;
  int posisiSudut;
};

#endif // GATE_SERVO_H
