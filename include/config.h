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

// (Opsional) Pin Relay Selenoid Valve jika ingin mematikan pompa secara otomatis
#define RELAY_VALVE_PIN         6

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
