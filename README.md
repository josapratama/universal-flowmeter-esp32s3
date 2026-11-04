# Universal Liquid Flow Meter ESP32-S3 + Sensor AICHI 1/2"

Sistem literan / flow meter digital multifluida berbasis **ESP32-S3 (N8R16)** dan sensor oval gear presisi tinggi **AICHI (1/2" - 2.5 mL/P)**. Sistem ini dirancang untuk dapat mengukur **berbagai jenis cairan** (Bahan Bakar, Minyak, Susu, Pelumas, Air, dll.) dengan dashboard Web Realtime yang responsif.

---

## 🌟 Fitur Utama Multifluida

1. **Dukungan Berbagai Jenis Cairan (Fluid Profiles):**
   - **Solar / Diesel (Bio Solar, Dexlite, Pertamina Dex)**
   - **Bensin / Gasoline (Pertalite, Pertamax, Turbo)**
   - **Minyak Goreng / Minyak Kelapa Sawit (Cooking Oil)**
   - **Oli Pelumas / Oli Mesin (Engine Oil)**
   - **Susu Segar / Dairy (Liquid Milk)**
   - **Minyak Tanah (Kerosene)**
   - **Air Bersih / Larutan Netral (Water)**
   - **Cairan Kustom:** Nama cairan, satuan (Liter/kg/mL), harga, dan K-Factor bebas ditentukan sendiri.

2. **Pengukuran Presisi Positif (Oval Gear 2.5 mL/P):**
   - Spesifikasi AICHI 1/2": 1 pulsa = 2.5 mL $\rightarrow$ K-Factor standar = **400 pulsa / Liter**.
   - Dilengkapi **Asisten Kalibrasi Mandiri** (Bejana Ukur / Timbangan) langsung dari Web UI.

3. **Dashboard Web Realtime Dinamis:**
   - Menyesuaikan label secara otomatis sesuai cairan yang aktif (misal: "PENGISIAN SUSU", "LITER", "TOTAL HARGA").
   - Menampilkan debit aliran (Satuan/menit dan Satuan/jam) secara realtime (update setiap 300 ms).
   - Odometer total volume seumur hidup tersimpan aman di Flash NVS.
3. **Kendali & Kalibrasi Digital:**
   - Tombol **[RESET / NOLKAN PENGISIAN]** untuk memulai pengisian baru (seperti tombol nol di SPBU).
   - **Kalkulator Kalibrasi Otomatis (Bejana Ukur):** Cukup masukkan volume bejana ukur asli (misal 5 L) dan pembacaan sensor (misal 4.85 L), sistem akan otomatis menghitung dan menyimpan K-Factor baru ke NVS.
   - Pengaturan harga solar per liter yang dapat diubah kapan saja.
4. **Standalone Access Point + Captive Portal:**
   - ESP32-S3 memancarkan hotspot WiFi sendiri (`SOLAR-METER-ESP32`), sehingga dapat digunakan langsung di lapangan tanpa membutuhkan router internet.
   - Buka browser dan arahkan ke `http://192.168.4.1`.
5. **Debounce Hardware & Software:**
   - Filter mikrodetik pada ISR untuk menangkal interferensi frekuensi tinggi / induksi elektromagnetik (EMI) dari dinamo pompa solar.

---

## 📂 Struktur Folder Proyek

```
solar-flowmeter-esp32s3/
├── platformio.ini         # Konfigurasi board ESP32-S3 N8R16 & library
├── include/
│   ├── config.h           # Definisi Pin GPIO, konstanta pulsa, WiFi AP
│   └── web_page.h         # Template Web UI responsif (HTML/CSS/JS di Flash PROGMEM)
├── src/
│   └── main.cpp           # Logika pembacaan interrupt, REST API, & NVS
├── WIRING.md              # Skema wiring lengkap dan panduan pemasangan pipa
└── README.md              # Dokumentasi & petunjuk penggunaan
```

---

## 🔌 Ringkasan Wiring Cepat

| Komponen | Pin Sensor / Modul | Pin ESP32-S3 N8R16 | Catatan |
| :--- | :--- | :--- | :--- |
| **AICHI OF05ZAT** | Merah (VCC) | **5V / VIN** | Tegangan kerja 3.5V – 24V |
| | Hitam (GND) | **GND** | Ground bersama |
| | Putih/Biru (Signal) | **GPIO 4** | Sinyal pulsa NPN Open Collector |
| **Resistor Pull-Up** | 4.7kΩ s/d 10kΩ | Di antara **GPIO 4** & **3.3V** | **Wajib** ke 3.3V (bukan 5V) |
| **LED Indikator** | Anoda (+) via R 220Ω | **GPIO 2** | Berkedip saat solar mengalir |

*Lihat panduan lengkap di [WIRING.md](WIRING.md).*

---

## 🚀 Cara Upload Menggunakan PlatformIO (VS Code)

1. Buka folder `project/solar-flowmeter-esp32s3` di **Visual Studio Code**.
2. Pastikan ekstensi **PlatformIO IDE** sudah terpasang di VS Code.
3. Hubungkan board **ESP32-S3 N8R16** ke komputer menggunakan kabel USB Type-C (ke port USB/UART ESP32).
4. Di bilah status bawah PlatformIO VS Code:
   - Klik ikon **Centang (Build)** untuk melakukan kompilasi.
   - Klik ikon **Panah Kanan (Upload)** untuk mengunggah program ke ESP32-S3.
   - Klik ikon **Stopkontak (Serial Monitor)** pada baud rate **115200**.

> Atau via terminal PlatformIO CLI:
> ```bash
> cd "project/solar-flowmeter-esp32s3"
> pio run --target upload
> pio device monitor -b 115200
> ```

---

## 📱 Cara Menghubungkan & Membuka Dashboard Web

1. Nyalakan ESP32-S3.
2. Pada smartphone atau laptop Anda, cari jaringan WiFi:
   - **SSID:** `SOLAR-METER-ESP32`
   - **Password:** `solar12345`
3. Setelah terhubung, buka browser (Chrome / Safari / Firefox) dan buka alamat:
   ```
   http://192.168.4.1
   ```
4. Dashboard literan solar akan langsung tampil secara realtime dengan refresh interval 300 ms!

---

## ⚖️ Cara Kalibrasi Pengukuran Menggunakan Bejana Ukur

Setiap instalasi pipa dan jenis solar (Biosolar, Dexlite, B35) dapat memiliki sedikit variasi resistansi viskositas:

1. Siapkan **Bejana Ukur Uji Tera** (misalnya ukuran 5 Liter atau 10 Liter).
2. Tekan tombol **[RESET / NOLKAN PENGISIAN]** di layar web.
3. Jalankan pompa sampai solar tepat mencapai tanda garis bejana ukur (misal 5.00 Liter).
4. Lihat angka di dashboard web (misal terbaca `4.850 Liter`).
5. Klik menu **[PENGATURAN]**:
   - Di bagian **Asisten Kalibrasi Mandiri**:
     - Isi Asli: `5.00`
     - Layar Baca: `4.85`
   - Klik tombol **[HITUNG & TERAPKAN K-FACTOR OTOMATIS]**.
   - Sistem akan otomatis menghitung K-Factor baru (`K_baru = K_lama * (sensor / actual)`).
6. Klik **[SIMPAN PENGATURAN]**. Nilai ini otomatis tersimpan secara permanen di memori flash NVS.

