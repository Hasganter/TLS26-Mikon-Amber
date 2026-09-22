#pragma once
#include <Arduino.h>
#include "../config/Config.h"

class GateServo;

class UltrasonicManager {
public:
  UltrasonicManager();

  // Inisialisasi pin trigger dan echo untuk sensor keluar (exit)
  void inisialisasi(const GateServo* gate = nullptr);
  void setGateServo(const GateServo* gate);

  // Polling sensor secara non-blocking
  void perbarui();

  // Dapatkan jarak terukur saat ini (cm)
  float getJarakKeluar() const;
  float getJarak() const;
  float getJarakKanan() const; // Kompatibilitas
  float getJarakKiri() const;  // Fallback (tidak ada sensor entrance)

  // Cek apakah ada kendaraan terdeteksi (< AMBANG_DETEKSI_CM)
  bool isMobilTerdeteksiKeluar() const;
  bool isMobilTerdeteksi() const;
  bool isMobilTerdeteksiKanan() const; // Kompatibilitas
  bool isMobilTerdeteksiKiri() const;  // Fallback (false)

  // Dapatkan status koneksi sensor
  bool isTerhubungKeluar() const;
  bool isTerhubung() const;
  bool isTerhubungKanan() const; // Kompatibilitas
  bool isTerhubungKiri() const;  // Fallback (false)

  // Berikan jeda waktu (ms) sebelum sensor aktif membaca kembali (mengatasi voltage drop servo)
  void tunda(unsigned long durasiMs);
  bool isDitunda() const;

private:
  float bacaSensor(uint8_t pinTrig, uint8_t pinEcho, bool &outTerhubung);

  unsigned long waktuSensorTerakhir;
  unsigned long waktuBolehBaca;
  float jarakKeluar;
  bool terhubungKeluar;
  const GateServo* gateServo;
};
