# Counter Parkiran Otomatis (Dual Lane)

Sistem penghitung dan pengatur slot parkir otomatis berbasis ESP32 dengan 2 lane (Lane Kiri untuk Masuk, Lane Kanan untuk Keluar), portal gate motor servo, layar OLED 128x64, keypad matriks 4x4, software RTC, serta penyimpanan permanen NVS (Non-Volatile Storage / Preferences).

---

## Komponen Hardware
1. **ESP32 DevKit C V4**
2. **Layar OLED 128x64 I2C (SSD1306)** — Antarmuka visual informasi parkir, jam, dan menu
3. **2x Sensor Ultrasonik HC-SR04**:
   - Sensor Lane Kiri: Deteksi kendaraan masuk & safety barrier
   - Sensor Lane Kanan: Deteksi kendaraan keluar
4. **Keypad Membran 4x4** — Tombol tiket, tombol navigasi, dan konfigurasi admin
5. **Servo Motor SG90** — Penggerak portal masuk (0° Tertutup, 90° Terbuka)
6. **6x Resistor 1kΩ** — Pembagi tegangan (Voltage Divider) pin ECHO 5V ke GPIO ESP32 3.3V (3 resistor per sensor)

---

## Parameter Sistem & Penyimpanan
- **Kapasitas Maksimal**: Default 5 slot (dapat diubah via debug mode).
- **Slot Tersedia**: Disimpan permanen di memori Flash ESP32 (`Preferences` / NVS), sehingga tidak hilang saat mati listrik atau restart.
- **Pencatatan Waktu**: Menggunakan Software RTC internal ESP32 (`time_t` / `struct tm`) dengan sinkronisasi dan edit waktu via keypad.

---

## Alur Kerja Sistem (Workflows)

### 1. Default Workflow 1 (Standby & Informasi)
Saat tidak ada interaksi di kedua lane, layar OLED berganti tampilan secara berkala setiap **30 detik**:
- **Tampilan A (30 detik)**:
  - Baris atas: "Selamat Datang"
  - Baris bawah: `Slot Parkir: {x}/{max}` (atau status `[PENUH]` jika x = 0)
- **Tampilan B (30 detik)**:
  - Baris atas: Waktu Real-Time (Hari, DD/MM/YYYY | 24h format HH:MM:SS)
  - Baris bawah: `Slot Parkir: {x}/{max}`
- Siklus berulang terus-menerus selama kondisi standby.

---

### 2. Penanganan Kondisi Khusus: Parkir Penuh (Slot = 0)
- Jika sisa slot = 0:
  - Layar menampilkan pemberitahuan: `[PARKIR PENUH!] Maaf, slot habis`.
  - Gate masuk **DIKUNCI** (tidak dapat dibuka baik via sensor lane kiri maupun tombol tiket).
  - Gate baru dapat dibuka kembali secara otomatis setelah ada kendaraan keluar dari lane kanan (slot > 0).
  - *Catatan:* Akses Debug Mode tetap dapat diakses oleh operator/admin untuk keperluan darurat atau override manual.

---

### 3. Default Workflow 2 (Tombol Tiket Masuk & Failsafe)
Alur ketika pengunjung menekan tombol keypad saat berada di gate masuk:
1. Pengunjung menekan tombol apapun pada keypad (selama slot > 0).
2. Layar OLED menampilkan hitung mundur pembatalan selama 5 detik:
   - Teks: `"Tunggu {x}s..."` dan `"Tekan C: Batal"`.
3. **Kondisi dalam jendela 5 detik:**
   - **Deteksi Sensor Kiri**: Jika sensor lane kiri mendeteksi keberadaan kendaraan, hitung mundur langsung dilewati dan gate **SEKETIKA TERBUKA**.
   - **Timeout (Failsafe)**: Jika 5 detik habis tanpa deteksi sensor dan tanpa pembatalan, gate tetap dibuka (masuk ke *Open Gate Workflow*).
   - **Pembatalan (`C`)**: Jika tombol `C` ditekan, alur dibatalkan dan sistem kembali ke Standby.
   - **Pintasan Debug Mode**: Jika ditekan kombinasi `Tombol Bebas + D + D + D` (tombol D ditekan 3 kali berturut-turut), sistem langsung masuk ke *Debug Mode Workflow* tanpa PIN.

---

### 4. Left Sensor Workflow (Lane Kiri / Lane Masuk)
1. Sensor ultrasonik lane kiri mendeteksi kendaraan mendekat:
   - Layar OLED menampilkan: `"Selamat Datang! Tekan Tombol Tiket"`.
2. **Respon Pengunjung:**
   - **Tombol Ditekan**: Masuk ke *Open Gate Workflow*.
   - **Tombol Tidak Ditekan**: Layar menahan pesan sambutan selama kendaraan masih terdeteksi di depan sensor.
   - **Kendaraan Pergi**: Jika kendaraan mundur/pergi tanpa menekan tombol, sistem kembali ke *Default Standby Workflow*.

---

### 5. Open Gate Workflow (Mekanisme Portal Masuk)
Alur pengoperasian motor servo portal masuk:
1. Motor servo berputar ke posisi **90°** (Portal Terbuka).
2. **Safety Hold**: Selama kendaraan masih terdeteksi oleh sensor ultrasonik lane kiri (sedang melintas di bawah portal), gate **TETAP TERBUKA** demi keamanan agar tidak menabrak kendaraan.
3. **Penutupan Aman (Vehicle Passed)**:
   - Setelah kendaraan selesai melintas (sensor kiri sudah tidak mendeteksi objek):
   - Sistem memulai hitung mundur aman selama **5 detik**.
   - Setelah 5 detik habis, servo berputar kembali ke **0°** (Portal Tertutup).
   - Jumlah slot parkir dikurangi (`slot = slot - 1`) dan nilai baru langsung disimpan ke NVS.
4. **Batas Waktu Pengaman (Safety Timeout 30 Detik)**:
   - Jika dalam 30 detik tidak ada kendaraan yang melintas:
     - Portal otomatis ditutup kembali (Servo ke 0°).
     - *Jika dibuka via deteksi sensor*: Slot **TIDAK DIKURANGI** (dianggap batal masuk).
     - *Jika dibuka via tombol failsafe (Workflow 2)*: Slot **TETAP DIKURANGI**.

---

### 6. Right Sensor Workflow (Lane Kanan / Lane Keluar)
Lane keluar beroperasi secara independen tanpa portal gate fisik:
1. Sensor ultrasonik lane kanan mendeteksi kendaraan yang hendak keluar area parkir.
2. Layar OLED menampilkan pesan ucapan:
   - `"Sampai Jumpa Lagi! Hati-hati di jalan"`.
3. Tampilan ditahan selama kendaraan masih berada di area deteksi sensor kanan.
4. Setelah kendaraan selesai melintas (sensor kanan sudah tidak mendeteksi objek):
   - Jumlah slot parkir ditambahkan (`slot = slot + 1`), dengan batas maksimum tidak melebihi kapasitas maksimal.
   - Nilai slot baru langsung disimpan ke NVS.
   - Sistem kembali ke *Default Standby Workflow*.

---

### 7. Debug Mode Workflow (Menu Konfigurasi & Override)
Akses khusus operator/admin untuk kalibrasi sistem secara mandiri:
- **Cara Masuk**: Tekan `Tombol Bebas + D + D + D` pada jendela pembatalan Workflow 2.
- **Menu Utama Debug**:
  - Layar OLED menampilkan:
    ```
    [ DEBUG MODE ]
    1:Waktu   2:Tgl
    3:Slot    C:Keluar
    ```
  - Navigasi Menu:
    - Tekan `1`: Sub-menu Edit Waktu
    - Tekan `2`: Sub-menu Edit Tanggal
    - Tekan `3`: Sub-menu Edit Slot Parkir Manual
    - Tekan `C`: Keluar dari Debug Mode (Kembali ke Standby)

#### Sub-menu 1: Edit Waktu (Time)
- Tampilan: `Set Jam (24h):` / `Format: HHMM`
- Input: 4 digit angka via keypad (0–9).
- Navigasi:
  - `*`: Hapus digit terakhir (Backspace).
  - `#`: Simpan waktu baru ke Software RTC.
  - `C`: Batalkan dan kembali ke Menu Utama Debug.
- Validasi: Format jam `00–23` dan menit `00–59`. Jika salah, tampilkan `"Format Invalid!"` lalu ulangi input.

#### Sub-menu 2: Edit Tanggal (Date)
- Tampilan: `Set Tanggal:` / `Format: DDMMYYYY`
- Input: 8 digit angka via keypad (0–9).
- Navigasi:
  - `*`: Hapus digit terakhir (Backspace).
  - `#`: Simpan tanggal baru ke Software RTC.
  - `C`: Batalkan dan kembali ke Menu Utama Debug.
- Validasi: Tanggal `01–31`, Bulan `01–12`, Tahun `2000–2099`. Jika salah, tampilkan `"Tanggal Invalid!"`.

#### Sub-menu 3: Edit Jumlah Slot Parkir Manual
- Tampilan:
  - Baris 1: `Slot: {x} / Max: {max}`
  - Baris 2: `A:+1 B:-1 #:Simpan`
- Metode Pengaturan:
  - **Inkremental**: Tekan `A` untuk menambah (+1) slot, tekan `B` untuk mengurangi (-1) slot.
  - **Input Angka Langsung**: Ketik angka baru via keypad (0 s/d Max).
- Navigasi:
  - `*`: Hapus / reset input.
  - `#`: Simpan jumlah slot baru ke NVS Flash.
  - `C`: Batalkan perubahan dan kembali ke Menu Utama Debug.
- Validasi: Nilai slot tidak boleh negatif (< 0) dan tidak boleh melebihi kapasitas maksimal (> max).

#### Fitur Pengaman Debug Mode
- **Timeout Otomatis (30 Detik)**: Jika tidak ada tombol yang ditekan selama 30 detik saat berada di dalam menu debug, sistem otomatis keluar dan kembali ke alur standby utama guna mencegah mode konfigurasi tertinggal terbuka.