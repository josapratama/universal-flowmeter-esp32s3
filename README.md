# ⛽ Smart Digital Flow Meter & Dispenser Controller (ESP32-S3 + Sensor AICHI)

Sistem literan digital multifluida & dispenser otomatis berbasis **ESP32-S3 (N8R16)** dan sensor oval gear presisi tinggi **AICHI (1/2" - 2.5 mL/P)**. Dilengkapi kontrol **Auto-Cutoff Solenoid Valve ala SPBU**, **Buku Log Riwayat 50 Transaksi & Export CSV**, **Dual WiFi (AP + Station)**, serta **Proteksi PIN Admin**.

---

## 🌟 Fitur Baru & Peningkatan Lengkap

### 1. 🎯 Dispenser Otomatis dengan Auto Cut-off Solenoid Valve (GPIO 6)
* **Preset Volume (Liter):** Pilihan cepat `1 L`, `2 L`, `5 L`, `10 L`, `20 L` atau input kustom.
* **Preset Nominal (Rupiah):** Pilihan cepat `Rp 20.000`, `Rp 50.000`, `Rp 100.000`, `Rp 200.000`.
* **Mode Manual / Penuh:** Aliran bebas dengan tombol kontrol manual.
* **Auto-Cutoff Akurat:** Saat target volume atau rupiah tercapai, relay selenoid valve otomatis memutus aliran seketika.
* **Audio Buzzer Feedback (GPIO 7):**
  - 1x Beep saat pengisian dimulai.
  - 3x Beep saat target tercapai (auto cut-off selesai).
* **Emergency Stop:** Tombol merah darurat untuk mematikan valve seketika kapan saja.

### 2. 📜 Buku Log Riwayat 50 Transaksi & Export CSV (Excel)
* Setiap transaksi pengisian yang selesai otomatis tercatat di Flash NVS:
  - No. ID Transaksi
  - Waktu Uptime
  - Jenis Cairan
  - Volume Pengisian
  - Total Biaya
  - Mode Pengisian
* Tombol **[📥 Download CSV (Excel)]** untuk mengunduh seluruh rekap transaksi ke smartphone/laptop tanpa aplikasi tambahan.

### 3. 🔐 Proteksi Keamanan PIN Admin (Default: `1234`)
* **Akses Operator:** Bebas memantau debit, memulai/menghentikan pengisian, dan reset batch.
* **Proteksi Admin (Wajib PIN):**
  - Pengubahan profil cairan & harga per liter.
  - Kalibrasi K-Factor sensor.
  - Nolkan total akumulasi seumur hidup (odometer).
  - Hapus riwayat transaksi.
  - Pengaturan koneksi WiFi rumah.

### 4. 📶 Dual WiFi Mode (AP Hotspot + Station LAN)
* Tetap memancarkan hotspot Access Point mandiri (`AICHI-FLOWMETER` di `192.168.4.1`) untuk penggunaan di lapangan.
* Dapat disambungkan ke WiFi rumah/kantor (STA Mode) sehingga dapat diakses langsung dari IP lokal (misal `192.168.1.xxx`) dan diintegrasikan ke **Omni Agent Hub**!

---

## 🔌 Ringkasan Pinout GPIO (ESP32-S3 N8R16)

| Komponen | Pin ESP32-S3 | Fungsi |
| :--- | :--- | :--- |
| **Sensor AICHI** | **GPIO 4** | Input pulsa sensor (wajib Pull-up 4.7k ke 3.3V) |
| **Relay Valve / Pompa** | **GPIO 6** | Output kontrol Solenoid Valve (Auto Cut-off) |
| **Buzzer Aktif 5V** | **GPIO 7** | Output audio alarm & notifikasi pengisian |
| **LED Indikator** | **GPIO 2** | Berkedip saat cairan mengalir |
| **I2C Display (Opsional)** | **GPIO 8 (SDA), GPIO 9 (SCL)** | Layar LCD 16x2 / OLED boks |

---

## 📡 REST API Endpoints (Untuk Integrasi IoT / Omni Hub)

| Method | Endpoint | Fungsi | Payload |
| :--- | :--- | :--- | :--- |
| `GET` | `/api/data` | Real-time telemetry (LPM, liter, valve, dll) | - |
| `POST` | `/api/dispense/start` | Memulai pengisian dengan target | `{"mode":"volume"\|"amount"\|"manual", "target": 5.0}` |
| `POST` | `/api/dispense/stop` | Hentikan pengisian (Emergency Stop) | - |
| `POST` | `/api/reset_batch` | Nolkan volume pengisian saat ini | - |
| `POST` | `/api/reset_total` | Nolkan odometer total seumur hidup | `{"pin":"1234"}` |
| `POST` | `/api/settings` | Simpan harga & K-Factor | `{"pin":"1234", "fluid_name":"...", "price":6800, "k_factor":400}` |
| `GET` | `/api/history` | Mengambil riwayat transaksi JSON | - |
| `GET` | `/api/history/export` | Download file `riwayat_transaksi.csv` | - |
| `POST` | `/api/history/clear` | Hapus seluruh riwayat transaksi | `{"pin":"1234"}` |
| `POST` | `/api/wifi/settings` | Simpan WiFi Rumah (STA) | `{"pin":"1234", "ssid":"...", "password":"..."}` |

---

## 🚀 Cara Upload Firmware ke Board ESP32-S3

1. Hubungkan ESP32-S3 ke komputer dengan kabel USB Type-C.
2. Buka terminal di folder project ini:
```bash
cd "D:\coding\raspberry pi\project\universal-flowmeter-esp32s3"
pio run --target upload
```
3. Hubungkan smartphone ke WiFi:
   - **SSID:** `AICHI-FLOWMETER`
   - **Password:** `meter12345`
   - **Buka Browser:** `http://192.168.4.1`
