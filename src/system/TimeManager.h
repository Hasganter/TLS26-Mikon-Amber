#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>

class TimeManager {
public:
  // Inisialisasi waktu awal software RTC (default: 09-09-2026 12:00:00 jika belum ada)
  static void inisialisasiWaktu();

  // Dapatkan string waktu (HH:MM:SS), tanggal (DD/MM/YYYY), dan nama hari bahasa Indonesia
  static void dapatkanWaktuFormat(char* bufWaktu, char* bufTanggal, char* bufHari);

  // Set waktu baru dari format HHMMSS (atau HHMM fallback)
  static bool setWaktuDariString(const String &str);

  // Set tanggal baru dari format DDMMYYYY
  static bool setTanggalDariString(const String &str);

private:
  static const char* NAMA_HARI[];
};

#endif // TIME_MANAGER_H
