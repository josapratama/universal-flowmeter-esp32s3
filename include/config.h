#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// PIN DEFINITIONS & LOGIC LEVELS
// ==========================================
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(ARDUINO_ESP32C3_DEV)
// Pinout ESP32-C3 Super Mini (13-pin ultra compact)
#define SENSOR_PIN              4  // Signal Pulsa AICHI (Pull-up 4.7k-10k ke 3.3V)
#define LED_FLOW_PIN            8  // Built-in Blue LED ESP32-C3 Super Mini
#define LED_ACTIVE_LEVEL        LOW // LED SuperMini aktif LOW (katoda terhubung ke GPIO 8)
#define RELAY_VALVE_PIN         6  // Relay Selenoid Valve
#define BUZZER_PIN              7  // Buzzer Alarm
#define I2C_SDA_PIN             2  // I2C SDA (OLED/LCD)
#define I2C_SCL_PIN             3  // I2C SCL (OLED/LCD)
#else
// Pinout ESP32-S3 DevKitC-1 (N8R16)
#define SENSOR_PIN              4
#define LED_FLOW_PIN            2
#define LED_ACTIVE_LEVEL        HIGH // LED ESP32-S3 aktif HIGH
#define RELAY_VALVE_PIN         6
#define BUZZER_PIN              7
#define I2C_SDA_PIN             8
#define I2C_SCL_PIN             9
#endif

// Logika Polaritas Relay (HIGH = Active High, LOW = Active Low)
#define RELAY_ACTIVE_LEVEL      HIGH
#define RELAY_INACTIVE_LEVEL    LOW

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

// Kalibrasi Aliran Multi-Titik (Multi-Point Calibration)
// Mengatasi non-linearitas sensor AICHI pada debit rendah (<5 LPM) vs debit tinggi (>15 LPM)
#define CALIB_LOW_LPM_LIMIT     5.0f
#define CALIB_HIGH_LPM_LIMIT    15.0f
#define DEFAULT_K_LOW           396.0f   // Aliran lambat (< 5 LPM)
#define DEFAULT_K_MID           400.0f   // Aliran normal (5 - 15 LPM)
#define DEFAULT_K_HIGH          404.0f   // Aliran deras (> 15 LPM)

// Filter Debounce Pulsa (mikrodetik)
// 200 us mencegah glitch / spike EMI dari motor pompa (max ~5000 Hz)
#define PULSE_DEBOUNCE_MICROS   200

// Inching / Overshoot Cut-off Compensation (Liter)
// Kompensasi keterlambatan penutupan katup mekanik & cairan di selang
#define DEFAULT_OVERSHOOT_LITERS 0.02f

// ==========================================
// PRESET BANK CAIRAN (BUILT-IN PRESETS)
// ==========================================
struct FluidPresetItem {
    const char* id;
    const char* name;
    float price;
    float kFactor;
    float overshoot;
    const char* emoji;
};

const FluidPresetItem BUILTIN_FLUIDS[] = {
    { "biosolar",       "Bio Solar Subsidi",   6800.0f,  400.0f, 0.020f, "⛽" },
    { "pertalite",      "Pertalite RON 90",    10000.0f, 400.0f, 0.020f, "⛽" },
    { "pertamax",       "Pertamax RON 92",     12100.0f, 400.0f, 0.020f, "⛽" },
    { "dexlite",        "Dexlite",             13050.0f, 400.0f, 0.020f, "⛽" },
    { "minyak_goreng",  "Minyak Goreng Curah", 15500.0f, 396.0f, 0.035f, "🛢️" },
    { "air_bersih",     "Air Bersih / RO",     50.0f,    400.0f, 0.010f, "💧" }
};
const size_t BUILTIN_FLUIDS_COUNT = sizeof(BUILTIN_FLUIDS) / sizeof(BUILTIN_FLUIDS[0]);

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
