# Sistem Counter & Portal Parkir Otomatis (Dual Lane) — ESP32

Sistem penghitung dan pengatur slot parkir otomatis berbasis ESP32 DevKit C V4 dengan dua jalur independen (Lane Kiri untuk Masuk berportal servo, Lane Kanan untuk Keluar tanpa portal), antarmuka grafis OLED 128x64 SSD1306 (I2C), keypad matriks membran 4x4, software RTC internal POSIX, helper pengetikan teks multi-tap T9 ala ponsel klasik, pemindai & pengelola koneksi WiFi asinkron, dan penyimpanan data permanen Non-Volatile Storage (NVS Flash) via library `Preferences`.

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
| *(Navigasi, Teks & Tiket)*| R2 | GPIO 12 | Output Scanning | Baris 2 matriks tombol |
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

Firmware disusun mengikuti arsitektur modular berorientasi domain di dalam folder `src/`:

```
TLS26-Mikon-Amber/
├── TLS26-Mikon-Amber.ino            # Orkestrator utama setup() & loop() (~120 baris)
├── concept.md                       # Spesifikasi konsep dan alur kerja sistem
├── diagram.json                     # Definisi simulasi visual dan pengkabelan Wokwi
├── wokwi.toml                       # File konfigurasi loader firmware Wokwi
├── libraries.txt                    # Dependensi library Wokwi
└── src/
    ├── config/
    │   ├── Config.h                 # Konfigurasi pinout, konstanta timing, batas slot 999
    │   └── SystemTypes.h            # Enum StatusSistem, InputMode, WiFiStatus, struct UIState
    ├── hardware/
    │   ├── GateServo.h/.cpp         # Driver pergerakan motor servo SG90
    │   ├── KeypadManager.h/.cpp     # Driver scanning matriks 4x4 keypad membran
    │   └── UltrasonicManager.h/.cpp # Polling non-blocking 2x sensor HC-SR04
    ├── system/
    │   ├── StorageManager.h/.cpp    # Driver persistensi NVS Flash (slot, max_slots, wifi)
    │   └── TimeManager.h/.cpp       # Driver Software RTC (POSIX struct tm, HHMMSS)
    ├── input/
    │   └── KeypadInputHelper.h/.cpp # Engine pengetikan teks Multi-Tap T9 (ABC/abc/123)
    ├── network/
    │   └── WiFiManager.h/.cpp       # Pengelola WiFi asinkron (scan, connect, reconnect)
    ├── ui/
    │   ├── DisplayManager.h/.cpp    # Core OLED driver, throttling I2C & dirty flag
    │   ├── UIHelper.h/.cpp          # Helper teks marquee scroll, header, footer
    │   ├── UIViewParking.h/.cpp     # View Standby, Countdown, Gate Terbuka, Lane Keluar
    │   └── UIViewDebug.h/.cpp       # View Menu, Waktu, Tanggal, Slot, WiFi Scan & Pass
    └── controller/
        ├── ParkingController.h/.cpp # FSM Alur Operasional Parkir Utama
        └── DebugController.h/.cpp   # FSM Menu Pengaturan & Koneksi WiFi
```

---

## 3. Parameter Sistem & Nilai Penyimpanan Dinamis (NVS Flash)

- **Kapasitas Maksimal (`kapasitasMaksimal`)**: Default `5` slot (`DEFAULT_KAPASITAS_MAKSIMAL`), dapat dikonfigurasi dinamis hingga **999** slot via NVS Flash (`namespace: "parking"`, `key: "max_slots"`).
- **Slot Tersedia Saat Ini (`slotTersedia`)**: Dimuat dan disimpan permanen pada NVS namespace `"parking"`, key `"slots"`.
- **Kredensial WiFi**: Disimpan permanen pada NVS namespace `"parking"`, key `"wifi_ssid"` dan `"wifi_pass"`.
- **Ambang Deteksi Jarak (`AMBANG_DETEKSI_CM`)**: `40.0 cm`.
- **Pewaktu Siklus Standby (`PERIODE_STANDBY_SCREEN`)**: `30000 ms` (30 detik).
- **Timeout Hitung Mundur Masuk (`TIMEOUT_COUNTDOWN_5S`)**: `5000 ms` (5 detik).
- **Timeout Penutupan Aman Setelah Lewat (`TIMEOUT_TUTUP_AMAN_5S`)**: `5000 ms` (5 detik).
- **Batas Maksimal Portal Terbuka (`TIMEOUT_GATE_OPEN_MAX`)**: `30000 ms` (30 detik).
- **Timeout Otomatis Menu Debug (`TIMEOUT_DEBUG_MS`)**: `30000 ms` (30 detik).
- **Jeda Multi-Tap T9 Commit (`MULTI_TAP_TIMEOUT_MS`)**: `800 ms`.
- **Batas Waktu Koneksi WiFi (`WIFI_CONNECT_TIMEOUT_MS`)**: `10000 ms` (10 detik).
- **Interval Polling Sensor Non-blocking (`INTERVAL_SENSOR_MS`)**: `60 ms`.

---

## 4. Finite State Machine (FSM) & Detail Alur Kerja

Status sistem diatur melalui enum `StatusSistem`:

### 1. `STATUS_STANDBY` (Dashboard Bersih & Slot Utama di Tengah)
- **Area Atas ($y = 2$)**: Judul status ringkas (Size 1)
  - Standby Mode A: `"SELAMAT DATANG"`
  - Standby Mode B: `"STATUS PARKIR"`
  - Kendaraan Mendekat: `"KENDARAAN MASUK"`
  - Parkir Penuh: `"! PARKIR PENUH !"`
- **Area Tengah ($y = 16\text{--}44$)**:
  - **Slot Tersedia (Normal)**:
    - Jika panjang teks slot $\le 5$ karakter (misal `5/5`, `50/50`): Font besar **Size 3** di tengah layar.
    - Jika panjang teks slot $> 5$ karakter (misal `150/999`, `999/999`): Auto-scaling ke font **Size 2** (lebar 84 px, rapi di tengah).
    - Dibingkai garis horizontal atas dan bawah ($y = 14$ dan $y = 45$).
  - **Kendaraan Mendekat (`jarakKiri < 40 cm`)**: Teks Size 2 tebal `"TIKET"`.
  - **Parkir Penuh (`slotTersedia <= 0`)**: Teks Size 3 besar `"PENUH"`.
- **Area Bawah ($y = 52$)**:
  - Standby Mode A: `"Tekan Tombol Tiket"`
  - Standby Mode B: Waktu real-time ringkas `Hari, HH:MM:SS` (misal `"Rabu, 21:45:00"`).
  - Parkir Penuh: `"Gerbang Dikunci"`

---

### 2. `STATUS_COUNTDOWN_5S` (Jendela Pembatalan & Pintasan Debug)
Dipicu saat pengunjung menekan tombol keypad apapun di lane masuk ketika slot tersedia:
- Header: `[ PROSES MASUK ]`
- Teks: `"Membuka gate dlm:"` dan angka detik besar di tengah.
- **Cabang Aksi:**
  1. **Fast-Bypass Sensor**: Jika sensor lane kiri mendeteksi kendaraan (`< 40 cm`), langsung membuka gate (`STATUS_GATE_TERBUKA`).
  2. **Pembatalan**: Tekan tombol `C` $\to$ kembali ke `STATUS_STANDBY`.
  3. **Pintasan Debug Mode**: Tekan `D` 3 kali berturut-turut (`Bebas + D + D + D`) $\to$ langsung beralih ke `STATUS_DEBUG_MENU`.
  4. **Timeout Failsafe (5 Detik)**: Gate otomatis dibuka jika tidak dibatalkan.

---

### 3. `STATUS_GATE_TERBUKA` (Portal Terbuka & Safety Hold)
- Motor servo bergerak ke sudut **90°** (terbuka).
- Header Layar: `[ PORTAL TERBUKA ]`
- **Safety Hold**: Selama mobil terdeteksi di bawah portal, penutupan ditahan.
- **Penutupan Aman**: Begitu mobil selesai lewat, hitung mundur aman 5 detik berjalan. Setelah 5 detik, servo kembali ke **0°**, slot berkurang 1, disimpan ke NVS Flash, dan kembali ke `STATUS_STANDBY`.
- **Safety Timeout (30 Detik)**: Jika mobil tidak lewat dalam 30 detik, gate otomatis menutup demi keamanan.

---

### 4. `STATUS_LANE_KELUAR` (Lane Kanan Otomatis)
Dipicu saat sensor ultrasonik lane kanan mendeteksi kendaraan keluar (`jarakKanan < 40 cm`):
- Header: `[ LANE KELUAR ]`
- Teks: `"SAMPAI JUMPA!"` (Size 2) dan `"Hati-hati di jalan!"`.
- Setelah mobil lewat: Slot parkir bertambah 1 (`slotTersedia++`, dibatasi maksimal `kapasitasMaksimal`), disimpan ke NVS Flash, lalu kembali ke `STATUS_STANDBY`.

---

### 5. `STATUS_DEBUG_MENU` (Menu Konfigurasi Operator)
Menu utama konfigurasi:
- Header: `--- DEBUG MODE ---`
- Pilihan Menu:
  - `1: Waktu  (HHMMSS)` $\to$ `STATUS_DEBUG_WAKTU`
  - `2: Tanggal(DDMMYYYY)` $\to$ `STATUS_DEBUG_TANGGAL`
  - `3: Slot Parkir` $\to$ `STATUS_DEBUG_SLOT`
  - `4: Koneksi WiFi` $\to$ `STATUS_DEBUG_WIFI_SCAN`
  - `C: Keluar ke Standby`
- **Auto Timeout 30 Detik**: Keluar otomatis jika idle selama 30 detik.

---

### 6. `STATUS_DEBUG_WAKTU` (Sub-Menu Jam Real-Time dengan Detik)
- Header: `[ SET WAKTU 24H ]`
- Menampilkan jam saat ini dan input 6 digit terformat: `[14:30:__]`.
- Input 4 digit (`HHMM`) tetap didukung sebagai fallback cerdas (detik otomatis diisi 00).
- Validasi: Jam 00–23, Menit 00–59, Detik 00–59.

---

### 7. `STATUS_DEBUG_TANGGAL` (Sub-Menu Tanggal)
- Header: `[ SET TANGGAL ]`
- Format: `DDMMYYYY`.
- Validasi: Tanggal 1–31, Bulan 1–12, Tahun 2000–2099.

---

### 8. `STATUS_DEBUG_SLOT` (Sub-Menu Edit Slot Parkir Terpadu hingga 999)
- Menampilkan nilai tersimpan saat ini (*previous value*) berdampingan dengan nilai baru yang sedang diketik:
  ```
  [ ATUR SLOT PARKIR ]
  --------------------
  >Tersedia: 4 -> [ 6 ]
   Maksimal: 5 -> [ 10]
  A:Pilih Field  *:Del
  #:Simpan       C:Batal
  ```
- Tombol `A`: Pindah kursor antara `Tersedia` dan `Maksimal`.
- Tombol `0–9`: Input angka hingga 3 digit (maksimal 999).
- Tombol `*`: Backspace.
- Tombol `#`: Simpan ($1 \le \text{Max} \le 999$ dan $0 \le \text{Slot} \le \text{Max}$).

---

### 9. `STATUS_DEBUG_WIFI_SCAN` (Pemindaian WiFi & Scrollable List)
- Menggunakan pemindaian asinkron `WiFi.scanNetworks(true)`.
- Menampilkan daftar SSID yang dapat di-scroll (3 baris per layar):
  - Item 0: `[+ Input Manual]` (untuk mengetik SSID hidden/khusus).
  - Item 1..n: `SSID` (+ tanda `*` jika terenkripsi).
- Tombol:
  - `A`: Scroll ke atas.
  - `B`: Scroll ke bawah.
  - `#`: Pilih item. Jika open WiFi, langsung konek; jika ber-password, buka layar input password.
  - `*`: Pindai ulang (*Rescan*).
  - `C`: Kembali ke menu utama debug.

---

### 10. `STATUS_DEBUG_WIFI_PASS` & `STATUS_DEBUG_WIFI_MANUAL` (Input Teks T9)
- Menggunakan helper `KeypadInputHelper` ala keypad ponsel jadul:
  - Tombol `0–9`: Multi-tap huruf/angka dengan jeda commit 800 ms.
  - Tombol `D`: Pengalih mode (`[ABC]` $\to$ `[abc]` $\to$ `[123]`).
  - Tombol `*`: Backspace.
  - Tombol `B`: Spasi cepat.
  - Tombol `#`: Selesai / Hubungkan.
  - Tombol `C`: Batal.
- Saat berhasil terhubung: Menampilkan IP Address (misal `192.168.1.50`) dan kredensial otomatis disimpan ke NVS Flash untuk auto-connect saat boot berikutnya.

---

## 5. Optimasi Efisiensi Pemrosesan

1. **OLED Frame Throttling**: Bus I2C hanya mentransfer buffer frame jika ada perubahan data/status (`isDirty`), saat jam berganti (500 ms), atau saat marquee scrolling aktif (dibatasi 33 ms / ~30 FPS). Ini memangkas beban CPU sebesar >80%.
2. **Asynchronous WiFi Scanning**: Pemindaian jaringan berjalan di latar belakang tanpa membekukan deteksi sensor ultrasonik atau pergerakan palang pintu.