#include "UltrasonicManager.h"
#include "GateServo.h"

UltrasonicManager::UltrasonicManager()
  : waktuSensorTerakhir(0),
    waktuBolehBaca(0),
    jarakKeluar(999.0f),
    terhubungKeluar(false),
    gateServo(nullptr) {}

void UltrasonicManager::inisialisasi(const GateServo* gate) {
  gateServo = gate;
  pinMode(PIN_TRIG_EXIT, OUTPUT);
  pinMode(PIN_ECHO_EXIT, INPUT_PULLDOWN);

  // Inisialisasi awal pembacaan sensor keluar jika gate tidak sedang bergerak
  if (!isDitunda()) {
    jarakKeluar = bacaSensor(PIN_TRIG_EXIT, PIN_ECHO_EXIT, terhubungKeluar);
  }
}

void UltrasonicManager::setGateServo(const GateServo* gate) {
  gateServo = gate;
}

void UltrasonicManager::tunda(unsigned long durasiMs) {
  waktuBolehBaca = millis() + durasiMs;
  jarakKeluar = 999.0f;
  Serial.printf("[ULTRASONIC] Sensor ditunda %lu ms untuk stabilisasi tegangan servo.\r\n", durasiMs);
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
  if (isDitunda()) {
    // Sedang dalam masa jeda stabilisasi tegangan saat/setelah servo bergerak
    jarakKeluar = 999.0f;
    return;
  }
  if (sekarang - waktuSensorTerakhir >= INTERVAL_SENSOR_MS) {
    waktuSensorTerakhir = sekarang;
    jarakKeluar = bacaSensor(PIN_TRIG_EXIT, PIN_ECHO_EXIT, terhubungKeluar);
  }
}

float UltrasonicManager::getJarakKeluar() const {
  return jarakKeluar;
}

float UltrasonicManager::getJarak() const {
  return jarakKeluar;
}

float UltrasonicManager::getJarakKanan() const {
  return jarakKeluar;
}

float UltrasonicManager::getJarakKiri() const {
  return 999.0f;
}

bool UltrasonicManager::isMobilTerdeteksiKeluar() const {
  if (isDitunda()) return false;
  return terhubungKeluar && (jarakKeluar < AMBANG_DETEKSI_CM);
}

bool UltrasonicManager::isDitunda() const {
  if (gateServo && gateServo->isSedangBergerak()) {
    return true;
  }
  return (millis() < waktuBolehBaca);
}

bool UltrasonicManager::isMobilTerdeteksi() const {
  return isMobilTerdeteksiKeluar();
}

bool UltrasonicManager::isMobilTerdeteksiKanan() const {
  return isMobilTerdeteksiKeluar();
}

bool UltrasonicManager::isMobilTerdeteksiKiri() const {
  return false;
}

bool UltrasonicManager::isTerhubungKeluar() const {
  return terhubungKeluar;
}

bool UltrasonicManager::isTerhubung() const {
  return terhubungKeluar;
}

bool UltrasonicManager::isTerhubungKanan() const {
  return terhubungKeluar;
}

bool UltrasonicManager::isTerhubungKiri() const {
  return false;
}
