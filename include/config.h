#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// PIN DEFINITIONS (ESP32-S3 N8R16)
// ==========================================
// Pin Sensor AICHI (NPN Open Collector)
// Hubungkan kabel Putih/Signal ke GPIO 4 dengan Pull-up resistor 4.7k-10k ke 3.3V
#define SENSOR_PIN              4

// Indikator LED (Blink saat cairan mengalir)
#define LED_FLOW_PIN            2

// Pin Relay Selenoid Valve (Auto Cut-off Pompa / Valve Pengisian)
#define RELAY_VALVE_PIN         6

// Pin Buzzer Alarm (Buzzer aktif 5V untuk notifikasi cut-off otomatis)
#define BUZZER_PIN              7

// Pin I2C Display (Opsional LCD 16x2 / OLED SSD1306)
#define I2C_SDA_PIN             8
#define I2C_SCL_PIN             9

// ==========================================
// SENSOR AICHI 1/2" SPECIFICATIONS
// ==========================================
// Spesifikasi: 1 Pulsa = 2.5 mL (0.0025 Liter)
// 1 Liter = 1000 mL / 2.5 mL = 400.0 pulsa / Liter
#define DEFAULT_K_FACTOR        400.0f

// Profil Default
#define DEFAULT_FLUID_NAME      "Solar / Diesel"
#define DEFAULT_UNIT_NAME       "Liter"
#define DEFAULT_CURRENCY        "Rp"
#define DEFAULT_PRICE_PER_UNIT  6800.0f

// Filter Debounce Pulsa (mikrodetik)
// 200 us mencegah glitch / spike EMI dari motor pompa (max ~5000 Hz)
#define PULSE_DEBOUNCE_MICROS   200

// Inching / Overshoot Cut-off Compensation (Liter)
// Kompensasi keterlambatan penutupan katup mekanik & cairan di selang
#define DEFAULT_OVERSHOOT_LITERS 0.02f

// ==========================================
// SAFETY & ALARM TIMEOUTS
// ==========================================
// Proteksi Dry-Run: Jika valve buka tapi aliran 0 LPM selama 6 detik, matikan valve
#define DRY_RUN_TIMEOUT_MS      6000

// Deteksi Kebocoran: Jika valve tutup tapi terdeteksi > 15 pulsa (~37.5 mL)
#define LEAKAGE_THRESHOLD_PULSES 15

// ==========================================
// NTP TIME SYNCHRONIZATION (WIB GMT+7)
// ==========================================
#define NTP_SERVER_1            "id.pool.ntp.org"
#define NTP_SERVER_2            "pool.ntp.org"
#define NTP_SERVER_3            "time.google.com"
#define GMT_OFFSET_SEC          (7 * 3600)  // WIB (UTC+7)
#define DAYLIGHT_OFFSET_SEC     0

// ==========================================
// SECURITY & ADMIN PIN
// ==========================================
#define DEFAULT_ADMIN_PIN       "1234"

// Maksimal transaksi disimpan di history
#define MAX_HISTORY_RECORDS     50

// ==========================================
// ACCESS POINT & WIFI DEFAULTS
// ==========================================
#define AP_SSID                 "AICHI-FLOWMETER"
#define AP_PASSWORD             "meter12345"
#define AP_IP_ADDRESS           "192.168.4.1"

// Web Server Port
#define HTTP_PORT               80

// Interval Penyimpanan Total ke NVS (milidetik)
#define NVS_AUTO_SAVE_INTERVAL  10000 

#endif // CONFIG_H
