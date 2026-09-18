#include "GateServo.h"

GateServo::GateServo()
  : pinPwm(PIN_SERVO_GATE),
    posisiSudut(SERVO_TUTUP),
    terhubung(false),
    waktuCekTerakhir(0) {}

bool GateServo::deteksiHardware() {
  bool wasAttached = servo.attached();
  if (wasAttached) {
    servo.detach();
  }

  // 1. Berikan muatan HIGH ke pin sejenak
  pinMode(pinPwm, OUTPUT);
  digitalWrite(pinPwm, HIGH);
  delayMicroseconds(20);

  // 2. Ubah ke INPUT High-Z murni
  pinMode(pinPwm, INPUT);
  delayMicroseconds(50);

  // Pin mengambang (tanpa servo) mempertahankan muatan HIGH.
  // Servo terhubung memiliki pull-down internal yang menguras muatan ke LOW.
  bool pinDischarged = (digitalRead(pinPwm) == LOW);

  // 3. Konfirmasi dengan INPUT_PULLUP
  pinMode(pinPwm, INPUT_PULLUP);
  delayMicroseconds(50);
  int adcVal = analogRead(pinPwm);
  // Pin mengambang tertarik penuh ke 3.3V (adcVal >= 4000)
  // Servo terhubung memiliki jalur pull-down sehingga adcVal < 3800
  bool loadPresent = (adcVal < 3800) || pinDischarged;

  // Pulihkan kembali PWM servo jika sebelumnya terpasang
  if (wasAttached) {
    servo.attach(pinPwm, 500, 2400);
    servo.write(posisiSudut);
  }

  terhubung = loadPresent;
  return terhubung;
}

void GateServo::inisialisasi(uint8_t pinServo) {
  pinPwm = pinServo;
  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);
  servo.attach(pinServo, 500, 2400);
  tutup();

  // Jalankan deteksi fisik awal
  deteksiHardware();
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

bool GateServo::isOk() {
  unsigned long sekarang = millis();
  if (sekarang - waktuCekTerakhir >= 1000) {
    waktuCekTerakhir = sekarang;
    deteksiHardware();
  }
  return terhubung;
}

int GateServo::getSudut() const {
  return posisiSudut;
}
