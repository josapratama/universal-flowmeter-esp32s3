# Panduan Wiring & Skema: Smart Flow Meter & Dispenser ESP32-S3 + Sensor AICHI

Dokumen ini menjelaskan detail pengkabelan, skema sirkuit, dan petunjuk mekanikal pemasangan sensor aliran solar **AICHI TOKEI**, relay solenoid valve auto-cutoff, buzzer, dan mikrokontroler **ESP32-S3 N8R16**.

---

## 1. Karakteristik Sensor AICHI
* **Prinsip Kerja:** *Oval Gear (Positive Displacement)* — Pengukuran akurat untuk fluida cair & kental (Solar, Bensin, Minyak Goreng, Oli, Susu, Air).
* **Tipe Output:** *NPN Open Collector Pulse*.
* **Tegangan Operasional:** DC 3.5V – 24V.
* **Konstanta Pulsa:** **2.5 mL / pulsa** (standar **400 pulsa / Liter**).
* **Rentang Aliran:** 5 – 200 Liter / jam.

---

## 2. Tabel Sambungan Lengkap Pin ESP32-S3

| Komponen | Pin Modul | Pin ESP32-S3 | Catatan Khusus |
| :--- | :--- | :--- | :--- |
| **Sensor AICHI** | Merah (VCC) | **5V / VIN** | Tegangan input DC 5V |
| | Hitam (GND) | **GND** | Ground bersama |
| | Putih/Biru (Signal) | **GPIO 4** | Pasang Pull-up Resistor ke 3.3V |
| **Resistor Pull-Up** | 4.7kΩ s/d 10kΩ | Antara **GPIO 4 & 3.3V** | **Wajib** ke 3.3V agar pin ESP32 aman |
| **Relay Valve / Pompa** | IN (Signal) | **GPIO 6** | Mengontrol Solenoid Valve / Pompa (Auto Cut-off) |
| | VCC & GND | **5V & GND** | Power modul relay |
| **Active Buzzer** | Anoda (+) | **GPIO 7** | Beep konfirmasi & alarm cut-off tercapai |
| | Katoda (-) | **GND** | Ground |
| **LED Indikator** | Anoda (+) via 220Ω | **GPIO 2** | Berkedip saat cairan mengalir |
| **(Opsional) I2C LCD/OLED** | SDA | **GPIO 8** | I2C Data Line |
| | SCL | **GPIO 9** | I2C Clock Line |

---

## 3. Diagram Skema Pengkabelan (Wiring Schematic)

```
        AICHI SENSOR                                ESP32-S3 N8R16
     +-------------------+                       +--------------------+
     |                   |                       |                    |
     |  KABEL MERAH  ----+---------------------->| 5V / VIN           |
     |                   |                       |                    |
     |  KABEL HITAM  ----+---------------------->| GND                |
     |                   |                       |                    |
     |  KABEL PUTIH  ----+----------*----------->| GPIO 4 (Pulse In)  |
     |  (Pulse Out)      |          |            |                    |
     +-------------------+          |            |                    |
                                    |            |                    |
                             [Resistor 4.7kΩ]    |                    |
                                    |            |                    |
                                    +----------->| 3.3V (3V3)         |
                                                 |                    |
     +-------------------+                       |                    |
     | RELAY VALVE (IN)  |<----------------------| GPIO 6 (Auto Cut)  |
     | BUZZER ALARM (+)  |<----------------------| GPIO 7 (Alarm Beep)|
     | LED ALIRAN (+)    |<----------------------| GPIO 2 (Flow Blink)|
     | I2C LCD (SDA)     |<----------------------| GPIO 8             |
     | I2C LCD (SCL)     |<----------------------| GPIO 9             |
     +-------------------+                       +--------------------+
```

---

## 4. Cara Kerja Auto-Cutoff Solenoid Valve
1. Operator memilih target pengisian (misal: 10 Liter atau Rp 50.000) di Web UI.
2. Saat tombol **[MULAI PENGISIAN]** ditekan:
   - ESP32 mengaktifkan `GPIO 6` (Relay ON $\rightarrow$ Solenoid Valve / Pompa terbuka).
   - Buzzer berbunyi 1 kali (beep singkat).
3. Selama pengisian, pulsa dihitung secara presisi setiap 300 ms.
4. Begitu volume mencapai 10 Liter (atau biaya mencapai Rp 50.000):
   - ESP32 langsung mematikan `GPIO 6` (Relay OFF $\rightarrow$ Solenoid Valve menutup seketika).
   - Buzzer berbunyi 3 kali (triple beep) menandakan pengisian selesai.
   - Transaksi otomatis tersimpan ke **Riwayat 50 Transaksi** di Flash NVS!
