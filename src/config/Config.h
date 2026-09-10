#pragma once
#include <Arduino.h>
#include "SystemTypes.h"

// KONFIGURASI PIN & HARDWARE
// ── Layar OLED 128x64 SSD1306 (I2C) ──────────────────────────
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define PIN_OLED_SDA 21
#define PIN_OLED_SCL 22

// ── Servo Motor SG90 (Gate Masuk) ─────────────────────────────
#define PIN_SERVO_GATE 4
const int SERVO_TUTUP = 0;   // Sudut tertutup (derajat)
const int SERVO_BUKA  = 90;  // Sudut terbuka (derajat)

// ── Sensor Ultrasonik HC-SR04 Lane Kiri (Masuk) ───────────────
#define PIN_TRIG_LEFT  5
#define PIN_ECHO_LEFT  18

// ── Sensor Ultrasonik HC-SR04 Lane Kanan (Keluar) ──────────────
#define PIN_TRIG_RIGHT 19
#define PIN_ECHO_RIGHT 23

// ── Keypad Membran 4x4 ────────────────────────────────────────
const byte KEYPAD_BARIS = 4;
const byte KEYPAD_KOLOM = 4;

const byte PIN_KEYPAD_BARIS[KEYPAD_BARIS] = { 13, 12, 14, 27 };  // R1, R2, R3, R4
const byte PIN_KEYPAD_KOLOM[KEYPAD_KOLOM] = { 26, 25, 33, 32 };  // C1, C2, C3, C4

// PARAMETER SISTEM & TIMING
const int DEFAULT_KAPASITAS_MAKSIMAL = 5;
const int BATAS_MAX_SLOT = 999;
const float AMBANG_DETEKSI_CM = 40.0f;  // Batas jarak deteksi kendaraan (cm)

// Konstanta Waktu (milidetik)
const unsigned long PERIODE_STANDBY_SCREEN = 30000; // 30 detik siklus tampilan standby
const unsigned long TIMEOUT_COUNTDOWN_5S   = 5000;  // 5 detik jendela pembatalan
const unsigned long TIMEOUT_TUTUP_AMAN_5S  = 5000;  // 5 detik penutupan aman setelah mobil lewat
const unsigned long TIMEOUT_GATE_OPEN_MAX  = 30000; // 30 detik batas waktu gate terbuka
const unsigned long TIMEOUT_DEBUG_MS       = 30000; // 30 detik timeout otomatis menu debug
const unsigned long INTERVAL_SENSOR_MS     = 60;    // Interval polling sensor non-blocking
const unsigned long MULTI_TAP_TIMEOUT_MS   = 800;   // 800 ms jeda commit karakter T9
const unsigned long WIFI_CONNECT_TIMEOUT_MS= 10000; // 10 detik batas waktu koneksi WiFi

// KONFIGURASI WEB SERVER & WEBSOCKET
const uint16_t HTTP_PORT = 80;
const uint16_t WS_PORT   = 81;
const char AP_FALLBACK_SSID[] = "TLS26-Parkir";
const char AP_FALLBACK_PASS[] = "adminparkir";

