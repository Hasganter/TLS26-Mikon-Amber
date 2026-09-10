#include "UltrasonicManager.h"

UltrasonicManager::UltrasonicManager()
  : waktuSensorTerakhir(0),
    giliranKiri(true),
    jarakKiri(999.0f),
    jarakKanan(999.0f) {}

void UltrasonicManager::inisialisasi() {
  pinMode(PIN_TRIG_LEFT, OUTPUT);
  pinMode(PIN_ECHO_LEFT, INPUT);
  pinMode(PIN_TRIG_RIGHT, OUTPUT);
  pinMode(PIN_ECHO_RIGHT, INPUT);
}

float UltrasonicManager::bacaSensor(uint8_t pinTrig, uint8_t pinEcho) {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  // Batasi timeout 12000 us (~2.0 meter) agar tidak memblokir loop utama
  unsigned long durasi = pulseIn(pinEcho, HIGH, 12000);
  if (durasi == 0) return 999.0f;
  return (durasi * 0.0343f) / 2.0f;
}

void UltrasonicManager::perbarui() {
  unsigned long sekarang = millis();
  if (sekarang - waktuSensorTerakhir >= INTERVAL_SENSOR_MS) {
    waktuSensorTerakhir = sekarang;
    if (giliranKiri) {
      jarakKiri = bacaSensor(PIN_TRIG_LEFT, PIN_ECHO_LEFT);
    } else {
      jarakKanan = bacaSensor(PIN_TRIG_RIGHT, PIN_ECHO_RIGHT);
    }
    giliranKiri = !giliranKiri;
  }
}

float UltrasonicManager::getJarakKiri() const {
  return jarakKiri;
}

float UltrasonicManager::getJarakKanan() const {
  return jarakKanan;
}

bool UltrasonicManager::isMobilTerdeteksiKiri() const {
  return (jarakKiri < AMBANG_DETEKSI_CM);
}

bool UltrasonicManager::isMobilTerdeteksiKanan() const {
  return (jarakKanan < AMBANG_DETEKSI_CM);
}
