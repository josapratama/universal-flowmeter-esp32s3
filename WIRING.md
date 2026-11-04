# Panduan Wiring & Skema: Literan Solar ESP32-S3 + Sensor AICHI OF05ZAT

Dokumen ini menjelaskan detail pengkabelan, skema sirkuit, dan petunjuk mekanikal pemasangan sensor aliran solar **AICHI TOKEI OF05ZAT** dengan mikrokontroler **ESP32-S3 N8R16**.

---

## 1. Karakteristik Sensor AICHI OF05ZAT

* **Prinsip Kerja:** *Oval Gear (Positive Displacement)* — Pengukuran akurat untuk fluida kental seperti Solar/Diesel, Dexlite, Oli, dan Minyak Tanah.
* **Tipe Output:** *NPN Open Collector Pulse*.
* **Tegangan Operasional:** DC 3.5V – 24V.
* **Konstanta Pulsa:** **0.46 mL / pulsa** (sekitar **2173.91 pulsa / Liter**).
* **Tekanan Maksimum:** 1.0 MPa (10 Bar).
* **Rentang Aliran:** 5 – 200 Liter / jam.

---

## 2. Arti Warna Kabel Sensor AICHI OF05ZAT

| Warna Kabel | Fungsi | Keterangan |
| :--- | :--- | :--- |
| **MERAH (Red)** | **VCC (Power +)** | Dihubungkan ke **5V / VIN** ESP32 atau adaptor DC 5V – 12V |
| **HITAM (Black)** | **GND (Power -)** | Dihubungkan ke **GND** ESP32 |
| **PUTIH / BIRU (White/Blue)** | **SIGNAL (Pulse Output)** | Dihubungkan ke **GPIO 4** ESP32-S3 + Resistor Pull-up |

> [!IMPORTANT]
> **PENTING: Sensor bertipe NPN Open Collector!**
> Karena outputnya open collector, Anda **WAJIB** memasang sebuah **Resistor Pull-Up (4.7 kΩ atau 10 kΩ)** di antara pin **SIGNAL (Kabel Putih)** dan pin **3.3V** ESP32.
> **JANGAN tarik pull-up ke 5V atau 12V**, agar tegangan logika yang masuk ke GPIO ESP32-S3 tetap aman pada **3.3V**!

---

## 3. Diagram Pengkabelan (Wiring Schematic)

```
        AICHI OF05ZAT                              ESP32-S3 N8R16
     +-------------------+                       +----------------+
     |                   |                       |                |
     |  KABEL MERAH  ----+---------------------->| 5V / VIN       |
     |                   |                       |                |
     |  KABEL HITAM  ----+---------------------->| GND            |
     |                   |                       |                |
     |  KABEL PUTIH  ----+----------*----------->| GPIO 4         |
     |  (Pulse Out)      |          |            |                |
     +-------------------+          |            |                |
                                    |            |                |
                             [Resistor 4.7kΩ]    |                |
                                    |            |                |
                                    +----------->| 3.3V (3V3)     |
                                                 |                |
     +-------------------+                       |                |
     | (Opsional) LED    |<----------------------| GPIO 2         |
     | (Opsional) RELAY  |<----------------------| GPIO 6         |
     +-------------------+                       +----------------+
```

### Rincian Sambungan Pin:

1. **VCC Sensor (Merah)** $\rightarrow$ **5V (VIN)** pada modul ESP32-S3.
2. **GND Sensor (Hitam)** $\rightarrow$ **GND** pada modul ESP32-S3.
3. **SIGNAL Sensor (Putih)** $\rightarrow$ **GPIO 4** pada ESP32-S3.
4. **Resistor Pull-Up 4.7 kΩ s/d 10 kΩ**:
   - Satu kaki resistor ke **GPIO 4 / Kabel Putih**.
   - Satu kaki resistor ke **Pin 3.3V (3V3)** ESP32-S3.
5. **(Opsional) Indikator Aliran LED**:
   - Anoda (+) LED melalui resistor 220Ω $\rightarrow$ **GPIO 2**.
   - Katoda (-) LED $\rightarrow$ **GND**.
6. **(Opsional) Modul Relay Pompa/Selenoid Valve**:
   - IN Relay $\rightarrow$ **GPIO 6**.
   - VCC Relay $\rightarrow$ **5V**.
   - GND Relay $\rightarrow$ **GND**.

---

## 4. Panduan Mekanikal Pipa Solar

Untuk menjaga akurasi pengukuran dan ketahanan sensor oval gear:

1. **Arah Aliran (Flow Direction):**
   Perhatikan tanda panah yang tercetak pada bodi sensor AICHI OF05ZAT. Fluida solar harus mengalir searah dengan panah tersebut.
2. **Wajib Pasang Filter / Saringan Solar (Strainer 60–100 Mesh):**
   Sensor oval gear bekerja dengan roda gigi presisi (celah mikron). Kotoran kerak tangki, serbuk gram besi, atau endapan lumpur solar dapat **mengganjal roda gigi**. Selalu pasang fuel filter sebelum sensor!
3. **Posisi Pemasangan:**
   - Pasang sensor pada **sisi dorong (tekanan/discharge) pompa**, bukan sisi hisap (suction).
   - Pastikan pipa terisi penuh solar (*liquid full condition*) dan bebas gelembung udara, karena gelembung udara akan ikut memutar roda gigi dan terbaca sebagai solar.
4. **Pencegahan Listrik Statis (Grounding):**
   Aliran bahan bakar minyak (solar) berpotensi memicu muatan elektrostatik. Pastikan pipa logam, tangki, dan sasis pompa dihubungkan ke tanah (*earth ground*).

