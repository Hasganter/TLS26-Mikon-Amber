#ifndef ULTRASONIC_MANAGER_H
#define ULTRASONIC_MANAGER_H

#include <Arduino.h>
#include "../config/Config.h"

class UltrasonicManager {
public:
  UltrasonicManager();

  // Inisialisasi pin trigger dan echo untuk kedua sensor
  void inisialisasi();

  // Polling sensor secara bergantian (non-blocking)
  void perbarui();

  // Dapatkan jarak terukur saat ini (cm)
  float getJarakKiri() const;
  float getJarakKanan() const;

  // Cek apakah ada kendaraan terdeteksi (< AMBANG_DETEKSI_CM)
  bool isMobilTerdeteksiKiri() const;
  bool isMobilTerdeteksiKanan() const;

private:
  float bacaSensor(uint8_t pinTrig, uint8_t pinEcho);

  unsigned long waktuSensorTerakhir;
  bool giliranKiri;
  float jarakKiri;
  float jarakKanan;
};

#endif // ULTRASONIC_MANAGER_H
