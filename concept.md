# Sistem Counter & Portal Parkir Otomatis (Dual Lane) — ESP32

Sistem penghitung dan pengatur slot parkir otomatis berbasis ESP32 DevKit C V4 dengan dua jalur independen (Lane Kiri untuk Masuk berportal servo, Lane Kanan untuk Keluar tanpa portal), antarmuka grafis OLED 128x64 SSD1306 (I2C), keypad matriks membran 4x4, software RTC internal POSIX, dan penyimpanan data permanen Non-Volatile Storage (NVS Flash) via library `Preferences`.

---

## 1. Arsitektur Perangkat Keras & Pemetaan Pin

| Modul / Komponen | Pin Modul | Pin ESP32 (GPIO) | Mode / Protokol | Fungsi & Catatan |
| :--- | :--- | :--- | :--- | :--- |
| **Layar OLED 128x64** | GND | GND.1 | Power | Ground display |
| *(SSD1306 I2C)* | VCC / VIN | 3V3 | Power | Suplai logika 3.3V |
| | SCL | GPIO 22 | I2C Clock | Clock default I2C ESP32 |
| | SDA | GPIO 21 | I2C Data | Data default I2C ESP32 (Addr: `0x3C`) |
| **Servo Portal SG90** | GND | GND.1 | Power | Ground motor servo |
| *(Gate Masuk)* | V+ | 5V | Power | Suplai daya 5V motor dari rail USB |
| | PWM | GPIO 4 | Output PWM | Kontrol sudut: 0° (Tutup) / 90° (Buka) |
| **HC-SR04 Lane Kiri** | VCC | 5V | Power | Suplai sensor 5V |
| *(Sensor Masuk & Safety)*| GND | GND.2 | Power | Ground sensor |
| | TRIG | GPIO 5 | Output Digital | Pulsa trigger ultrasonik 10 µs |
| | ECHO | Divider ke GPIO 18 | Input Pulsa | Pembagi tegangan $R_1, R_2, R_3$ (5V $\to$ 3.33V) |
| **HC-SR04 Lane Kanan** | VCC | 5V | Power | Suplai sensor 5V |
| *(Sensor Keluar)* | GND | GND.2 | Power | Ground sensor |
| | TRIG | GPIO 19 | Output Digital | Pulsa trigger ultrasonik 10 µs |
| | ECHO | Divider ke GPIO 23 | Input Pulsa | Pembagi tegangan $R_4, R_5, R_6$ (5V $\to$ 3.33V) |
| **Keypad Membran 4x4** | R1 | GPIO 13 | Output Scanning | Baris 1 matriks tombol |
| *(Navigasi & Tiket)* | R2 | GPIO 12 | Output Scanning | Baris 2 matriks tombol |
| | R3 | GPIO 14 | Output Scanning | Baris 3 matriks tombol |
| | R4 | GPIO 27 | Output Scanning | Baris 4 matriks tombol |
| | C1 | GPIO 26 | Input Pull-up | Kolom 1 matriks tombol |
| | C2 | GPIO 25 | Input Pull-up | Kolom 2 matriks tombol |
| | C3 | GPIO 33 | Input Pull-up | Kolom 3 matriks tombol |
| | C4 | GPIO 32 | Input Pull-up | Kolom 4 matriks tombol |
| **6x Resistor 1kΩ** | $R_1, R_2, R_3$ | Antara ECHO Kiri & GND | Pasif | Divider 5V $\times \frac{2k}{1k+2k} = 3.33V$ ke GPIO 18 |
| *(Voltage Divider)* | $R_4, R_5, R_6$ | Antara ECHO Kanan & GND | Pasif | Divider 5V $\times \frac{2k}{1k+2k} = 3.33V$ ke GPIO 23 |

---

## 2. Struktur Perangkat Lunak & Modularitas Kode

Firmware disusun mengikuti arsitektur modular terpisah untuk memisahkan domain logika:

```
TLS26-Mikon-Amber/
├── TLS26-Mikon-Amber.ino            # Orkestrator utama setup() & loop() finite state machine
├── concept.md                       # Spesifikasi konsep dan alur kerja sistem
├── diagram.json                     # Definisi simulasi visual dan pengkabelan Wokwi
├── wokwi.toml                       # File konfigurasi loader firmware Wokwi
├── libraries.txt                    # Dependensi library Wokwi
└── src/
    ├── DisplayManager.cpp           # Engine visual rendering grafis OLED 128x64 & auto-scroll helper
    ├── GateServo.cpp                # Driver pergerakan motor servo SG90
    ├── KeypadManager.cpp            # Driver scanning matriks 4x4 keypad membran
    ├── StorageManager.cpp           # Driver persistensi NVS Flash (Preferences)
    ├── TimeManager.cpp              # Driver Software RTC (POSIX struct tm / time_t)
    ├── UltrasonicManager.cpp        # Polling non-blocking 2x sensor HC-SR04
    └── include/
        ├── Config.h                 # Konfigurasi pinout, konstanta waktu, dan enum StatusSistem
        ├── DisplayManager.h
        ├── GateServo.h
        ├── KeypadManager.h
        ├── StorageManager.h
        ├── TimeManager.h
        └── UltrasonicManager.h
```

---

## 3. Parameter Sistem & Nilai Penyimpanan Dinamis (NVS Flash)

- **Kapasitas Maksimal (`kapasitasMaksimal`)**: Default `5` slot (`DEFAULT_KAPASITAS_MAKSIMAL`), disimpan dan dapat dikonfigurasi dinamis via NVS Flash (`namespace: "parking"`, `key: "max_slots"`).
- **Slot Tersedia Saat Ini (`slotTersedia`)**: Dimuat dan disimpan permanen pada NVS namespace `"parking"`, key `"slots"`.
- **Ambang Deteksi Jarak (`AMBANG_DETEKSI_CM`)**: `40.0 cm`.
- **Pewaktu Siklus Standby (`PERIODE_STANDBY_SCREEN`)**: `30000 ms` (30 detik).
- **Timeout Hitung Mundur Masuk (`TIMEOUT_COUNTDOWN_5S`)**: `5000 ms` (5 detik).
- **Timeout Penutupan Aman Setelah Lewat (`TIMEOUT_TUTUP_AMAN_5S`)**: `5000 ms` (5 detik).
- **Batas Maksimal Portal Terbuka (`TIMEOUT_GATE_OPEN_MAX`)**: `30000 ms` (30 detik).
- **Timeout Otomatis Menu Debug (`TIMEOUT_DEBUG_MS`)**: `30000 ms` (30 detik).
- **Interval Polling Sensor Non-blocking (`INTERVAL_SENSOR_MS`)**: `60 ms`.

---

## 4. Finite State Machine (FSM) & Detail Alur Kerja

Status sistem diatur melalui enum `StatusSistem` dengan 8 state:

### 1. `STATUS_STANDBY` (Dashboard Bersih & Slot Utama di Tengah)
Header pemisah ditiadakan agar layar bersih dan fokus pada informasi ketersediaan slot:
- **Area Atas ($y = 2$)**: Judul status ringkas (Size 1)
  - Standby Mode A: `"SELAMAT DATANG"`
  - Standby Mode B: `"STATUS PARKIR"`
  - Kendaraan Mendekat: `"KENDARAAN MASUK"`
  - Parkir Penuh: `"! PARKIR PENUH !"`
- **Area Tengah ($y = 16\text{--}44$)**:
  - **Slot Tersedia (Normal)**: Menampilkan kartu angka besar **Size 3** di tengah layar (misal `3/5`), dibingkai garis horizontal atas dan bawah ($y = 14$ dan $y = 45$).
  - **Kendaraan Mendekat (`jarakKiri < 40 cm`)**: Teks Size 2 tebal `"TEKAN TIKET"`.
  - **Parkir Penuh (`slotTersedia <= 0`)**: Teks Size 3 besar `"PENUH"`.
- **Area Bawah ($y = 52$)**:
  - Standby Mode A: `"Tekan Tombol Tiket"`
  - Standby Mode B: Waktu real-time ringkas `Hari, HH:MM:SS` (misal `"Rabu, 21:45:00"`).
  - Parkir Penuh: `"Gerbang Dikunci"`

---

### 2. `STATUS_COUNTDOWN_5S` (Jendela Pembatalan & Pintasan Debug)
Dipicu saat pengunjung menekan tombol keypad apapun di lane masuk ketika slot tersedia:
- Layar menampilkan hitung mundur besar dari 5 ke 1 detik:
  - Header: `[ PROSES MASUK ]`
  - Teks: `"Membuka gate dalam:"`
  - Angka hitung mundur (Size 3 di tengah)
  - Baris bawah: `"Tekan C : Batalkan"`
- **Cabang Aksi:**
  1. **Fast-Bypass Sensor**: Jika sensor lane kiri mendeteksi kendaraan (`< 40 cm`), hitung mundur seketika dilewati dan sistem langsung melompat ke `STATUS_GATE_TERBUKA`.
  2. **Pembatalan Pengunjung**: Jika tombol `C` ditekan, alur dibatalkan dan sistem kembali ke `STATUS_STANDBY`.
  3. **Pintasan Debug Mode**: Jika tombol ditekan berurutan `(Bebas) + D + D + D` (tombol D ditekan 3 kali berturut-turut), sistem langsung beralih ke `STATUS_DEBUG_MENU`.
  4. **Timeout Failsafe (5 Detik)**: Jika 5 detik habis tanpa pembatalan, portal tetap dibuka otomatis (`STATUS_GATE_TERBUKA`) dengan penanda `gateDibukaLewatSensor = false`.

---

### 3. `STATUS_GATE_TERBUKA` (Portal Terbuka & Safety Hold)
- Motor servo bergerak ke sudut **90°** (terbuka).
- Header Layar: `[ PORTAL TERBUKA ]`
- **Safety Hold**:
  - Selama sensor kiri mendeteksi kendaraan melintas di bawah portal (`jarakKiri < 40 cm`), layar menampilkan `"Mobil Sedang Lewat"` & `"MASUK"` (Size 2).
  - Timer penutupan ditahan (portal tidak akan menutup selama mobil masih di bawah portal).
- **Penutupan Aman (Mobil Telah Lewat)**:
  - Begitu sensor kiri tidak lagi mendeteksi objek, sistem memulai hitung mundur aman **5 detik**.
  - Layar menampilkan `"Mobil Selesai Lewat"` dan hitung mundur `"Tutup Gate dalam: {s}s"`.
  - Jika 5 detik selesai tanpa ada halangan baru: servo kembali ke **0°** (tutup), slot berkurang 1 (`slotTersedia--`), nilai baru disimpan permanen ke NVS, dan kembali ke `STATUS_STANDBY`.
- **Safety Timeout (30 Detik)**:
  - Jika mobil tidak pernah lewat dalam 30 detik: servo kembali ke **0°**.
  - Jika gate dibuka via failsafe tombol (tanpa mobil terdeteksi), slot tetap dikurangi. Jika dibuka via sensor tapi mobil batal lewat, slot tidak dikurangi.

---

### 4. `STATUS_LANE_KELUAR` (Lane Kanan Otomatis)
Dipicu saat sensor ultrasonik lane kanan mendeteksi kendaraan keluar (`jarakKanan < 40 cm`):
- Header Layar: `[ LANE KELUAR ]`
- Teks utama: `"SAMPAI JUMPA!"` (Size 2) dan baris bawah `"Hati-hati di jalan!"` (Size 1).
- Ketika kendaraan telah selesai melintas (sensor kanan kembali bebas `> 40 cm`):
  - Slot parkir bertambah 1 (`slotTersedia++`), dibatasi maksimal `kapasitasMaksimal`.
  - Nilai slot baru disimpan ke NVS Flash.
  - Delay feedback visual 800 ms, lalu kembali ke `STATUS_STANDBY`.

---

### 5. `STATUS_DEBUG_MENU` (Menu Konfigurasi Operator)
Menu utama konfigurasi:
- Header: `--- DEBUG MODE ---`
- Daftar Menu:
  - `1: Waktu  (HHMM)` $\to$ `STATUS_DEBUG_WAKTU`
  - `2: Tanggal(DDMMYYYY)` $\to$ `STATUS_DEBUG_TANGGAL`
  - `3: Slot Parkir` $\to$ `STATUS_DEBUG_SLOT`
  - `C: Keluar ke Standby`
- **Auto Timeout 30 Detik**: Jika tidak ada penekanan tombol selama 30 detik, sistem otomatis keluar ke `STATUS_STANDBY`.

---

### 6. `STATUS_DEBUG_WAKTU` (Sub-Menu Jam Real-Time)
- Header: `[ SET WAKTU (24h) ]`
- Menampilkan jam saat ini dan input 4 digit: `[HHMM]`.
- Tombol `0–9`: Memasukkan digit waktu.
- Tombol `*`: Backspace (hapus digit terakhir).
- Tombol `C`: Batal dan kembali ke menu debug.
- Tombol `#`: Simpan ke Software RTC.
  - Validasi: Jam 00–23, Menit 00–59.
  - Feedback visual: `"Waktu Disimpan!"` atau `"Waktu Invalid!"` selama 2 detik.
- Baris bawah: `"*:Hapus #:Ok C:Batal"` (bebas overflow).

---

### 7. `STATUS_DEBUG_TANGGAL` (Sub-Menu Tanggal)
- Header: `[ SET TANGGAL ]`
- Menampilkan tanggal saat ini dan input 8 digit: `[DDMMYYYY]`.
- Tombol `0–9`: Memasukkan digit tanggal.
- Tombol `*`: Backspace.
- Tombol `C`: Batal dan kembali ke menu debug.
- Tombol `#`: Simpan ke Software RTC.
  - Validasi: Tanggal 1–31, Bulan 1–12, Tahun 2000–2099.
  - Feedback visual: `"Tanggal Disimpan!"` atau `"Tanggal Invalid!"` selama 2 detik.
- Baris bawah: `"*:Hapus #:Ok C:Batal"` (bebas overflow).

---

### 8. `STATUS_DEBUG_SLOT` (Sub-Menu Edit Slot Parkir Terpadu)
Layar konfigurasi terpadu untuk mengubah slot tersedia dan kapasitas maksimal secara bersamaan:
- Header: `[ ATUR SLOT PARKIR ]`
- Tampilan Field:
  - Baris 1: `> Tersedia: [ x ]` (atau spasi jika tidak aktif)
  - Baris 2: `  Maksimal: [ y ]` (atau `>` jika aktif)
- Navigasi & Input:
  - **Tombol `A`**: Beralih kursor/fokus aktif antara field `Tersedia` dan `Maksimal`.
  - **Tombol `0–9`**: Mengetik nilai angka pada field aktif (maks 2 digit).
  - **Tombol `*`**: Menghapus digit terakhir (*backspace*) pada field aktif.
  - **Tombol `#`**: Memvalidasi dan menyimpan kedua nilai ke Flash NVS.
    - *Aturan Validasi:* $1 \le \text{Maksimal} \le 99$ dan $0 \le \text{Tersedia} \le \text{Maksimal}$.
    - *Feedback Visual:* `"Slot Disimpan!"` jika valid, atau `"Nilai Invalid!"` jika salah.
  - **Tombol `C`**: Membatalkan perubahan dan kembali ke menu utama debug.

---

## 5. Fitur Auto-Scroll Horizontal Text Ticker

Untuk mencegah teks terpotong di tepi kanan layar OLED 128x64:
- Engine `DisplayManager::drawTextScroll` secara otomatis memeriksa panjang piksel teks ($L \times 6 \times \text{Size}$).
- Jika panjang teks $\le 128\text{px}$, teks dicetak statis (tengah/rapi).
- Jika panjang teks $> 128\text{px}$, sistem menjalankan animasi geser horizontal mulus (*marquee*) dengan jeda 1.2 detik di awal sebelum mulai bergeser pada kecepatan 35 ms per piksel.