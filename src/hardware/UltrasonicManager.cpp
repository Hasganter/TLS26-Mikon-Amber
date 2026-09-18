#include "UltrasonicManager.h"

UltrasonicManager::UltrasonicManager()
  : waktuSensorTerakhir(0),
    giliranKiri(true),
    jarakKiri(999.0f),
    jarakKanan(999.0f),
    terhubungKiri(false),
    terhubungKanan(false) {}

void UltrasonicManager::inisialisasi() {
  pinMode(PIN_TRIG_LEFT, OUTPUT);
  pinMode(PIN_ECHO_LEFT, INPUT_PULLDOWN);
  pinMode(PIN_TRIG_RIGHT, OUTPUT);
  pinMode(PIN_ECHO_RIGHT, INPUT_PULLDOWN);

  // Inisialisasi awal pembacaan kedua sensor
  jarakKiri = bacaSensor(PIN_TRIG_LEFT, PIN_ECHO_LEFT, terhubungKiri);
  delay(10);
  jarakKanan = bacaSensor(PIN_TRIG_RIGHT, PIN_ECHO_RIGHT, terhubungKanan);
}

float UltrasonicManager::bacaSensor(uint8_t pinTrig, uint8_t pinEcho, bool &outTerhubung) {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);

  // Cek apakah pin echo sudah HIGH sebelum trigger (pin floating / shorted)
  if (digitalRead(pinEcho) == HIGH) {
    outTerhubung = false;
    return 999.0f;
  }

  // Kirim pulsa trigger 10 mikrosekon
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  // Tunggu respon pulsa HIGH dari sensor HC-SR04 (timeout 1500 us)
  unsigned long t0 = micros();
  while (digitalRead(pinEcho) == LOW) {
    if (micros() - t0 > 1500) {
      // Tidak ada respon pulsa sama sekali -> Sensor tidak terhubung!
      outTerhubung = false;
      return 999.0f;
    }
  }

  // Sensor merespon dengan pulsa -> Sensor terhubung!
  outTerhubung = true;

  // Ukur durasi pulsa HIGH (timeout 12000 us = ~2.0 meter)
  unsigned long tHigh = micros();
  while (digitalRead(pinEcho) == HIGH) {
    if (micros() - tHigh > 12000) {
      // Objek berada di luar jangkauan 2 meter
      return 999.0f;
    }
  }

  unsigned long durasi = micros() - tHigh;
  if (durasi < 100) return 999.0f;
  return (durasi * 0.0343f) / 2.0f;
}

void UltrasonicManager::perbarui() {
  unsigned long sekarang = millis();
  if (sekarang - waktuSensorTerakhir >= INTERVAL_SENSOR_MS) {
    waktuSensorTerakhir = sekarang;
    if (giliranKiri) {
      jarakKiri = bacaSensor(PIN_TRIG_LEFT, PIN_ECHO_LEFT, terhubungKiri);
    } else {
      jarakKanan = bacaSensor(PIN_TRIG_RIGHT, PIN_ECHO_RIGHT, terhubungKanan);
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
  return terhubungKiri && (jarakKiri < AMBANG_DETEKSI_CM);
}

bool UltrasonicManager::isMobilTerdeteksiKanan() const {
  return terhubungKanan && (jarakKanan < AMBANG_DETEKSI_CM);
}

bool UltrasonicManager::isTerhubungKiri() const {
  return terhubungKiri;
}

bool UltrasonicManager::isTerhubungKanan() const {
  return terhubungKanan;
}
