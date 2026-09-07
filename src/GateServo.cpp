#include "include/GateServo.h"

GateServo::GateServo() : posisiSudut(SERVO_TUTUP) {}

void GateServo::inisialisasi(uint8_t pinServo) {
  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);
  servo.attach(pinServo, 500, 2400);
  tutup();
}

void GateServo::buka() {
  posisiSudut = SERVO_BUKA;
  servo.write(SERVO_BUKA);
}

void GateServo::tutup() {
  posisiSudut = SERVO_TUTUP;
  servo.write(SERVO_TUTUP);
}

bool GateServo::isTerbuka() const {
  return (posisiSudut == SERVO_BUKA);
}
