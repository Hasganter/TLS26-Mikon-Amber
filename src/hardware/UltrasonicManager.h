#pragma once
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

  // Dapatkan status koneksi sensor
  bool isTerhubungKiri() const;
  bool isTerhubungKanan() const;

private:
  float bacaSensor(uint8_t pinTrig, uint8_t pinEcho, bool &outTerhubung);

  unsigned long waktuSensorTerakhir;
  bool giliranKiri;
  float jarakKiri;
  float jarakKanan;
  bool terhubungKiri;
  bool terhubungKanan;
};

