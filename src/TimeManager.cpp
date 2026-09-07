#include "include/TimeManager.h"

const char* TimeManager::NAMA_HARI[] = {
  "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"
};

void TimeManager::inisialisasiWaktu() {
  time_t now;
  time(&now);
  struct tm *ti = localtime(&now);

  // Jika tahun masih sebelum 2026, set waktu default ke 09-09-2026 12:00:00 (Rabu)
  if (ti->tm_year + 1900 < 2026) {
    struct tm tAwal = {0};
    tAwal.tm_year = 2026 - 1900;
    tAwal.tm_mon  = 9 - 1;       // September (0-11)
    tAwal.tm_mday = 9;
    tAwal.tm_hour = 12;
    tAwal.tm_min  = 0;
    tAwal.tm_sec  = 0;

    struct timeval tv;
    tv.tv_sec = mktime(&tAwal);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
  }
}

void TimeManager::dapatkanWaktuFormat(char* bufWaktu, char* bufTanggal, char* bufHari) {
  time_t now;
  time(&now);
  struct tm *ti = localtime(&now);

  sprintf(bufWaktu, "%02d:%02d:%02d", ti->tm_hour, ti->tm_min, ti->tm_sec);
  sprintf(bufTanggal, "%02d/%02d/%04d", ti->tm_mday, ti->tm_mon + 1, ti->tm_year + 1900);
  strcpy(bufHari, NAMA_HARI[ti->tm_wday]);
}

bool TimeManager::setWaktuDariString(const String &str) {
  if (str.length() != 4) return false;
  int jam = str.substring(0, 2).toInt();
  int mnt = str.substring(2, 4).toInt();

  if (jam < 0 || jam > 23 || mnt < 0 || mnt > 59) return false;

  time_t now;
  time(&now);
  struct tm *ti = localtime(&now);
  ti->tm_hour = jam;
  ti->tm_min  = mnt;
  ti->tm_sec  = 0;

  struct timeval tv;
  tv.tv_sec = mktime(ti);
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
  return true;
}

bool TimeManager::setTanggalDariString(const String &str) {
  if (str.length() != 8) return false;
  int tgl = str.substring(0, 2).toInt();
  int bln = str.substring(2, 4).toInt();
  int thn = str.substring(4, 8).toInt();

  if (tgl < 1 || tgl > 31 || bln < 1 || bln > 12 || thn < 2000 || thn > 2099) return false;

  time_t now;
  time(&now);
  struct tm *ti = localtime(&now);
  ti->tm_mday = tgl;
  ti->tm_mon  = bln - 1;
  ti->tm_year = thn - 1900;

  struct timeval tv;
  tv.tv_sec = mktime(ti);
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
  return true;
}
