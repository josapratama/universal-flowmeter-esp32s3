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
#define RELAY_VALVE_PIN         6  // Relay Selenoid Valve (Katup Aliran Utama 100%)
#define RELAY_VALVE_SLOW_PIN    10 // Relay Selenoid Valve (Katup Inching/Aliran Pelan 20%)
#define BUZZER_PIN              7  // Buzzer Alarm
#define I2C_SDA_PIN             2  // I2C SDA (OLED/LCD)
#define I2C_SCL_PIN             3  // I2C SCL (OLED/LCD)
#define THERMAL_SENSOR_PIN      5  // DS18B20 1-Wire Temperature Sensor (Opsional)
#else
// Pinout ESP32-S3 DevKitC-1 (N8R16)
#define SENSOR_PIN              4
#define LED_FLOW_PIN            2
#define LED_ACTIVE_LEVEL        HIGH // LED ESP32-S3 aktif HIGH
#define RELAY_VALVE_PIN         6  // Relay Selenoid Valve (Katup Aliran Utama 100%)
#define RELAY_VALVE_SLOW_PIN    5  // Relay Selenoid Valve (Katup Inching/Aliran Pelan 20%)
#define BUZZER_PIN              7
#define I2C_SDA_PIN             8
#define I2C_SCL_PIN             9
#define THERMAL_SENSOR_PIN      18 // DS18B20 1-Wire Temperature Sensor (Opsional)
#endif

// Logika Polaritas Relay (HIGH = Active High, LOW = Active Low)
#define RELAY_ACTIVE_LEVEL      HIGH
#define RELAY_INACTIVE_LEVEL    LOW

// ==========================================
// TWO-STAGE VALVE CONTROL (INCHING FLOW)
// ==========================================
// Mode Katup 2-Tahap: Stage 1 = Aliran Utama 100%, Stage 2 = Aliran Pelan 20% saat mendekati target
#define DEFAULT_TWO_STAGE_ENABLED       true
#define DEFAULT_SLOW_STAGE_LITERS       0.25f // 250 mL sebelum target, matikan katup utama, buka katup pelan

// ==========================================
// SMART TANK INVENTORY (LEVEL MONITORING)
// ==========================================
#define DEFAULT_TANK_CAPACITY_L         1000.0f
#define DEFAULT_TANK_REMAINING_L        850.0f
#define DEFAULT_TANK_LOW_ALERT_L        100.0f

// ==========================================
// MQTT TELEMETRY & HOME ASSISTANT DISCOVERY
// ==========================================
#define DEFAULT_MQTT_ENABLED            false
#define DEFAULT_MQTT_BROKER             "192.168.100.184"
#define DEFAULT_MQTT_PORT               1883
#define DEFAULT_MQTT_USER               ""
#define DEFAULT_MQTT_PASS               ""
#define DEFAULT_MQTT_TOPIC              "flowmeter/telemetry"
#define DEFAULT_MQTT_CLIENT_ID          "aichi-flowmeter"

// ==========================================
// THERMAL EXPANSION COMPENSATION (15°C / 20°C)
// ==========================================
#define DEFAULT_TEMP_COMP_ENABLED       false
#define DEFAULT_REFERENCE_TEMP_C        15.0f
#define DEFAULT_THERMAL_EXP_COEFF       0.00095f // 0.095% per °C untuk Solar/Diesel (0.00125 untuk Bensin)

// ==========================================
// ESC/POS THERMAL RECEIPT PRINTER
// ==========================================
#define DEFAULT_STORE_NAME              "SPBU MINI PERTAMINI"
#define DEFAULT_STORE_ADDRESS           "JL. RAYA UTAMA NO. 88"
#define DEFAULT_STORE_FOOTER            "TERIMA KASIH - SEMOGA SELAMAT SAMPAI TUJUAN"

// ==========================================
// I2C LCD DISPLAY CONFIGURATION (20x4 / 16x2)
// ==========================================
#define LCD_COLS                20
#define LCD_ROWS                4
#define LCD_DEFAULT_I2C_ADDR    0x27  // Alamat I2C PCF8574 default (0x27 atau 0x3F)

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
