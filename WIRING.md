# Panduan Wiring & Skema Lengkap: Smart Flow Meter & Dispenser (ESP32-C3 SuperMini & ESP32-S3)

Dokumen ini menjelaskan detail pengkabelan (*wiring*), skema sirkuit, pinout fisik, dan panduan proteksi kelistrikan untuk menghubungkan sensor aliran solar/minyak **AICHI TOKEI**, modul relay solenoid valve auto-cutoff, buzzer alarm, display I2C, serta mikrokontroler **ESP32-C3 SuperMini** dan **ESP32-S3 DevKitC-1**.

---

## 1. Karakteristik & Spesifikasi Sensor AICHI
* **Prinsip Kerja:** *Oval Gear (Positive Displacement)* — sangat akurat untuk fluida cair maupun kental (Bio Solar, Pertalite, Pertamax, Dexlite, Minyak Goreng Curah, Oli, Susu, Air).
* **Tipe Output:** *NPN Open Collector Pulse* (memerlukan resistor pull-up eksternal ke 3.3V).
* **Tegangan Operasional:** DC 3.5V – 24V (Direkomendasikan **5V** dari pin 5V/VBUS).
* **Konstanta Pulsa:** **2.5 mL / pulsa** (standar **400 pulsa / Liter**).
* **Rentang Aliran:** 5 – 200 Liter / jam (0.08 – 3.33 LPM, hingga 30 LPM pada pompa dorong).

---

## 2. Tabel Perbandingan Sambungan Pin (ESP32-C3 vs ESP32-S3)

Firmware ini mendukung deteksi target otomatis saat kompilasi (`CONFIG_IDF_TARGET_ESP32C3` / `ARDUINO_ESP32C3_DEV`):

| Komponen / Modul | Pin Modul | Sambungan ke ESP32-C3 SuperMini | Sambungan ke ESP32-S3 DevKitC-1 | Keterangan & Catatan Teknis |
| :--- | :--- | :--- | :--- | :--- |
| **Sensor AICHI VCC** | Merah | **5V / VBUS** | **5V / VIN** | Tegangan input DC 5V sensor |
| **Sensor AICHI GND** | Hitam | **GND** | **GND** | Ground bersama (*common ground*) |
| **Sensor AICHI Signal** | Putih / Biru | **GPIO 4** | **GPIO 4** | Input pulsa flowmeter |
| **Resistor Pull-Up** | 4.7kΩ s/d 10kΩ | Antara **GPIO 4 & 3V3** | Antara **GPIO 4 & 3V3** | ⚠️ **Wajib ke 3.3V** (jangan ke 5V agar pin aman) |
| **Relay Valve / Pompa** | IN (Signal) | **GPIO 6** | **GPIO 6** | Kontrol Solenoid Valve / Pompa (Auto Cut-off) |
| | VCC & GND | **5V & GND** | **5V & GND** | Daya modul relay 5V |
| **Active Buzzer** | Positif (+) | **GPIO 7** | **GPIO 7** | Indikator beep & alarm keselamatan |
| | Negatif (-) | **GND** | **GND** | Ground |
| **LED Indikator Aliran** | Built-in / Eksternal | **GPIO 8 (Built-in Blue)** | **GPIO 2** | C3: LED biru bawaan (Aktif LOW)<br>S3: GPIO 2 via R 220Ω |
| **Layar LCD 20x4 / 16x2 (I2C)**| VCC | **5V / VBUS** | **5V / VIN** | ⚠️ **Wajib 5V** agar kristal LCD menyala tajam! |
| | GND | **GND** | **GND** | Ground bersama |
| | SDA (Data) | **GPIO 2** | **GPIO 8** | Jalur data I2C |
| | SCL (Clock) | **GPIO 3** | **GPIO 9** | Jalur clock I2C |
| **(Alternatif) OLED 0.96"**| VCC & GND | **3V3 & GND** | **3V3 & GND** | Khusus OLED SSD1306 (tegangan 3.3V) |
| | SDA & SCL | **GPIO 2 & GPIO 3** | **GPIO 8 & GPIO 9** | Jalur data & clock I2C |

---

## 3. Diagram Pinout Fisik ESP32-C3 SuperMini

ESP32-C3 SuperMini adalah board ultra kompak (16 pin, ukuran 22.5 mm × 18 mm). Berikut adalah tata letak pin fisiknya:

```
                  +-------------------+
                  |     [USB-C]       |
                  |                   |
           3V3 ---| [ ] 3V3     5V [ ]|--- 5V / VBUS (Input 5V)
           GND ---| [ ] GND    GND [ ]|--- GND
                  | [ ] GPIO0  IO6 [ ]|--- GPIO 6 (Relay Selenoid Valve)
                  | [ ] GPIO1  IO7 [ ]|--- GPIO 7 (Buzzer Alarm)
(I2C SDA)  IO2 ---| [ ] GPIO2  IO8 [ ]|--- GPIO 8 (LED Biru Onboard / Flow)
(I2C SCL)  IO3 ---| [ ] GPIO3  IO9 [ ]|--- GPIO 9 (Tombol BOOT)
(Pulsa)    IO4 ---| [ ] GPIO4 IO10 [ ]|--- GPIO 10
                  | [ ] GPIO5             |
                  +-------------------+
```

---

## 4. Diagram Skema Pengkabelan Lengkap: ESP32-C3 SuperMini

```
         SENSOR AICHI (1/2")                       ESP32-C3 SUPERMINI
      +------------------------+                 +---------------------+
      |                        |                 |                     |
      |   KABEL MERAH (VCC)    +---------------->| 5V / VBUS           |
      |                        |                 |                     |
      |   KABEL HITAM (GND)    +---------------->| GND                 |
      |                        |                 |                     |
      |   KABEL PUTIH (Pulse)  +--------*------->| GPIO 4 (Pulse Input)|
      +------------------------+        |        |                     |
                                        |        |                     |
                                [Resistor 4.7kΩ] |                     |
                                        |        |                     |
                                        +------->| 3V3 (Output 3.3V)   |
                                                 |                     |
      +------------------------+                 |                     |
      | MODUL RELAY VALVE      |                 |                     |
      |   - Pin IN             |<----------------| GPIO 6 (Auto Cutoff)|
      |   - Pin VCC & GND      |<--- [5V & GND] -+                     |
      +------------------------+                 |                     |
                                                 |                     |
      +------------------------+                 |                     |
      | ACTIVE BUZZER 5V       |                 |                     |
      |   - Pin (+)            |<----------------| GPIO 7 (Buzzer Beep)|
      |   - Pin (-)            +---------------->| GND                 |
      +------------------------+                 |                     |
                                                 |                     |
      +------------------------+                 |                     |
      | (OPSIONAL) I2C OLED    |                 |                     |
      |   - SDA (Data)         |<----------------| GPIO 2              |
      |   - SCL (Clock)        |<----------------| GPIO 3              |
      |   - VCC (3.3V) & GND   |<--- [3V3 & GND]-+                     |
      +------------------------+                 |                     |
                                                 | [LED Biru di GPIO 8]|
                                                 +---------------------+
```

---

## 5. Diagram Skema Pengkabelan: ESP32-S3 DevKitC-1

```
         SENSOR AICHI (1/2")                          ESP32-S3 N8R16
      +------------------------+                 +---------------------+
      |   KABEL MERAH (VCC)    +---------------->| 5V / VIN            |
      |   KABEL HITAM (GND)    +---------------->| GND                 |
      |   KABEL PUTIH (Pulse)  +--------*------->| GPIO 4 (Pulse In)   |
      +------------------------+        |        |                     |
                                 [Resistor 4.7k] |                     |
                                        +------->| 3V3                 |
      +------------------------+                 |                     |
      | RELAY VALVE (IN)       |<----------------| GPIO 6 (Auto Cutoff)|
      | BUZZER ALARM (+)       |<----------------| GPIO 7 (Alarm Beep) |
      | LED INDIKATOR (+)      |<----------------| GPIO 2 (via 220Ω)   |
      | I2C LCD/OLED SDA       |<----------------| GPIO 8              |
      | I2C LCD/OLED SCL       |<----------------| GPIO 9              |
      +------------------------+                 +---------------------+
```

---

## 6. Diagram Penyambungan Solenoid Valve & Beban Pompa Listrik

Relay berfungsi sebagai saklar pemutus arus pada solenoid valve 12V/24V atau motor pompa 220V:

```
                    +--------------------------------+
                    | SUMBER ADAPTOR 12V / 24V DC   |
                    | (+) POSITIF       (-) NEGATIF  |
                    +------+-------------------+-----+
                           |                   |
                           v                   |
             +-------------+------------+      |
             |   MODUL RELAY 1 KANAL    |      |
             | [COM]              [NO]  |      |
             +---+------------------+---+      |
                 | (Arus Masuk)     | (Arus Keluar saat ON)
                 |                  v
                 |           +------+-----------------+
                 |           | SOLENOID VALVE 12V/24V |
                 |           |                        |
                 |           |  [+]             [-]   |
                 |           +---+---------------+----+
                 |               |               ^
                 |               |  (Flyback)    |
                 |               +-|>|-----------+
                 |                 Dioda 1N4007
                 |                               |
                 +-------------------------------+
```

> [!WARNING]
> **Pasang Dioda Flyback (1N4007):** Jika menggunakan Solenoid Valve DC (12V atau 24V), **wajib memasang dioda 1N4007 secara paralel terbalik** (Katoda garis perak ke `+`, Anoda ke `-`) langsung di terminal koil solenoid valve. Dioda ini menyerap lonjakan induktif (*back-EMF*) yang timbul saat katup menutup seketika, mencegah ESP32 me-restart sendiri akibat gangguan listrik.

---

## 7. Cara Kerja Otomatis Auto-Cutoff Dispenser
1. Operator memilih target pengisian di Web UI (misalnya: **10 Liter** atau **Rp 50.000**).
2. Saat tombol **[MULAI PENGISIAN]** diklik:
   - ESP32 mengaktifkan `GPIO 6` (Relay `HIGH` $\rightarrow$ Solenoid Valve terbuka / pompa menyala).
   - Buzzer berbunyi 1 kali (beep konfirmasi).
3. Selama cairan mengalir:
   - Sensor AICHI menghasilkan 400 pulsa per liter.
   - LED Indikator berkedip sinkron dengan putaran pulsa sensor.
   - Display Web menampilkan volume, biaya rupiah, speedometer LPM, dan estimasi waktu selesai (ETA) secara *realtime*.
4. Fitur Inching / Lead Compensation:
   - Dengan parameter kompensasi inersia (default **20 mL** / `0.02 L`), ESP32 menutup katup tepat sebelum volume meluap di selang dispenser.
5. Saat target tercapai:
   - ESP32 mematikan `GPIO 6` seketika (Relay `LOW` $\rightarrow$ katup tertutup rapat).
   - Buzzer berbunyi 3 kali (triple beep) dan browser memainkan melodi chime penutupan.
   - Transaksi otomatis tersimpan permanen di memori Flash NVS ESP32 dan dikirim ke Omni Agent Hub via Webhook.

---

## 8. Panduan Troubleshooting Wiring Lapangan

| Gejala Masalah | Kemungkinan Penyebab | Solusi Perbaikan |
| :--- | :--- | :--- |
| **Cairan mengalir tapi volume di web tetap 0.000 L** | Resistor pull-up belum terpasang atau salah pasang | Pastikan ada resistor 4.7kΩ – 10kΩ antara kabel sinyal (GPIO 4) dan 3.3V ESP32. |
| **ESP32 tiba-tiba restart / hang saat valve menutup** | Lonjakan induktif (*back-EMF*) koil solenoid | Pasang dioda 1N4007 pada koil valve, dan gunakan catu daya adaptor 12V/24V terpisah dari 5V ESP32 (GND disatukan). |
| **Angka literan melompat drastis (kebanyakan pulsa)** | Gangguan elektromagnetik (*noise EMI*) dari dinamo pompa | Jauhkan kabel sinyal sensor dari kabel daya 220V motor pompa; gunakan kabel *shielded* jika jarak > 1 meter. |
| **Status valve terbuka tapi relay tidak menyala** | Modul relay bertipe *Active Low* | Ubah `#define RELAY_ACTIVE_LEVEL LOW` pada `include/config.h` lalu flash ulang firmware. |
| **LED Biru SuperMini selalu menyala terang** | Polaritas LED C3 SuperMini | Built-in LED ESP32-C3 SuperMini menggunakan logika *Active Low* (katoda di GPIO 8), firmware sudah otomatis mengaturnya. |
| **LCD 20x4 I2C layar kosong / hanya lampu biru menyala (tanpa teks)** | Potensiometer (Trimpot) kontras belum disetel | **Putar trimpot biru (potensiometer obeng min)** di bagian belakang modul ransel I2C (PCF8574) hingga karakter teks hitam/putih terlihat tajam. |
| **LCD 20x4 teks redup atau tidak muncul sama sekali** | Tegangan VCC LCD diberi 3.3V | Layar LCD karakter HD44780 20x4 **memerlukan daya 5V** untuk polarisasi kristal cairnya. Hubungkan VCC LCD ke pin **5V / VBUS** ESP32, bukan ke 3V3! |
| **LCD 20x4 menampilkan kotak-kotak hitam tebal penuh** | Kontras terlalu tinggi atau pin SDA/SCL tertukar | Putar balik trimpot kontras ke arah sebaliknya perlahan. Pastikan pin SDA di **GPIO 2** dan SCL di **GPIO 3** (untuk ESP32-C3). Firmware sudah dilengkapi pemindai I2C otomatis (`0x27` & `0x3F`). |
