#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "config.h"
#include "web_page.h"

// ==========================================
// GLOBALS & OBJECTS
// ==========================================
WebServer server(HTTP_PORT);
DNSServer dnsServer;
Preferences preferences;

// Status & Perhitungan Flow Meter
volatile uint32_t isrPulseCount = 0;
volatile uint32_t isrBatchPulses = 0;
volatile uint32_t lastPulseMicros = 0;

float kFactor = DEFAULT_K_FACTOR;
float pricePerLiter = DEFAULT_PRICE_PER_UNIT;
String fluidName = DEFAULT_FLUID_NAME;
String unitName = DEFAULT_UNIT_NAME;
String currencySymbol = DEFAULT_CURRENCY;

float currentFlowRateLpm = 0.0f;
float batchLiters = 0.0f;
float batchCost = 0.0f;
double totalLiters = 0.0;

unsigned long lastCalcTime = 0;
unsigned long lastPulseSeenMillis = 0;
unsigned long lastNvsSaveMillis = 0;
double lastSavedTotalLiters = 0.0;
bool isDispensing = false;

// ==========================================
// INTERRUPT SERVICE ROUTINE (AICHI SENSOR)
// ==========================================
void IRAM_ATTR onFlowSensorPulse() {
    uint32_t now = micros();
    // Debounce filter untuk menolak spike EMI motor pompa
    if (now - lastPulseMicros >= PULSE_DEBOUNCE_MICROS) {
        isrPulseCount = isrPulseCount + 1;
        isrBatchPulses = isrBatchPulses + 1;
        lastPulseMicros = now;
    }
}

// ==========================================
// PERSISTENT STORAGE (NVS PREFERENCES)
// ==========================================
void loadSettingsFromNVS() {
    preferences.begin("flowmeter", false);
    totalLiters = preferences.getDouble("total_l", 0.0);
    kFactor = preferences.getFloat("k_factor", DEFAULT_K_FACTOR);
    pricePerLiter = preferences.getFloat("price_l", DEFAULT_PRICE_PER_UNIT);
    fluidName = preferences.getString("fluid", DEFAULT_FLUID_NAME);
    unitName = preferences.getString("unit", DEFAULT_UNIT_NAME);
    currencySymbol = preferences.getString("curr", DEFAULT_CURRENCY);
    preferences.end();

    lastSavedTotalLiters = totalLiters;
    Serial.printf("[NVS] Data dimuat: Cairan=%s, Total=%.2f %s, K-Factor=%.3f, Harga=%s %.0f\n", 
                  fluidName.c_str(), totalLiters, unitName.c_str(), kFactor, currencySymbol.c_str(), pricePerLiter);
}

void saveTotalToNVS() {
    if (abs(totalLiters - lastSavedTotalLiters) > 0.01) {
        preferences.begin("flowmeter", false);
        preferences.putDouble("total_l", totalLiters);
        preferences.end();
        lastSavedTotalLiters = totalLiters;
        Serial.printf("[NVS] Total akumulasi tersimpan: %.3f %s\n", totalLiters, unitName.c_str());
    }
}

void saveConfigToNVS() {
    preferences.begin("flowmeter", false);
    preferences.putFloat("k_factor", kFactor);
    preferences.putFloat("price_l", pricePerLiter);
    preferences.putString("fluid", fluidName);
    preferences.putString("unit", unitName);
    preferences.putString("curr", currencySymbol);
    preferences.end();
    Serial.println("[NVS] Konfigurasi profil cairan berhasil disimpan.");
}

// ==========================================
// HTTP SERVER HANDLERS
// ==========================================
void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void handleApiData() {
    JsonDocument doc;
    doc["fluid_name"] = fluidName;
    doc["unit_name"] = unitName;
    doc["currency"] = currencySymbol;
    doc["batch_liters"] = batchLiters;
    doc["batch_cost"] = batchCost;
    doc["flow_rate_lpm"] = currentFlowRateLpm;
    doc["total_liters"] = totalLiters;
    doc["batch_pulses"] = isrBatchPulses;
    doc["k_factor"] = kFactor;
    doc["price_per_liter"] = pricePerLiter;
    doc["is_dispensing"] = isDispensing;

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void handleResetBatch() {
    noInterrupts();
    isrBatchPulses = 0;
    interrupts();
    batchLiters = 0.0f;
    batchCost = 0.0f;

    Serial.println("[FLOW] Batch berhasil di-reset (NOLKAN VOLUME).");
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Batch direset ke 0\"}");
}

void handleResetTotal() {
    noInterrupts();
    isrBatchPulses = 0;
    interrupts();
    batchLiters = 0.0f;
    batchCost = 0.0f;
    totalLiters = 0.0;
    lastSavedTotalLiters = 0.0;

    preferences.begin("flowmeter", false);
    preferences.putDouble("total_l", 0.0);
    preferences.end();

    Serial.println("[FLOW] Total akumulasi seumur hidup berhasil dinolkan!");
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Total akumulasi direset ke 0\"}");
}

void handleSaveSettings() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Payload kosong\"}");
        return;
    }

    String body = server.arg("plain");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);

    if (err) {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Format JSON salah\"}");
        return;
    }

    if (doc["fluid_name"].is<String>()) {
        fluidName = doc["fluid_name"].as<String>();
    }
    if (doc["unit_name"].is<String>()) {
        unitName = doc["unit_name"].as<String>();
    }
    if (doc["currency"].is<String>()) {
        currencySymbol = doc["currency"].as<String>();
    }
    if (doc["price"].is<float>()) {
        pricePerLiter = doc["price"].as<float>();
    }
    if (doc["k_factor"].is<float>()) {
        float newK = doc["k_factor"].as<float>();
        if (newK > 0.0f) {
            kFactor = newK;
        }
    }

    saveConfigToNVS();
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Pengaturan tersimpan\"}");
}

// Captive portal redirect
void handleNotFound() {
    server.sendHeader("Location", String("http://") + AP_IP_ADDRESS, true);
    server.send(302, "text/plain", "");
}

// ==========================================
// ARDUINO SETUP
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==============================================");
    Serial.println("  UNIVERSAL LIQUID FLOW METER | AICHI SENSOR");
    Serial.println("==============================================");

    // Setup GPIO
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(LED_FLOW_PIN, OUTPUT);
    digitalWrite(LED_FLOW_PIN, LOW);

#ifdef RELAY_VALVE_PIN
    pinMode(RELAY_VALVE_PIN, OUTPUT);
    digitalWrite(RELAY_VALVE_PIN, LOW); // Solenoid valve off
#endif

    // Muat data tersimpan dari Flash NVS
    loadSettingsFromNVS();

    // Attach hardware interrupt ke sensor AICHI
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), onFlowSensorPulse, FALLING);
    Serial.printf("[SENSOR] Interrupt terpasang pada GPIO %d (FALLING edge)\n", SENSOR_PIN);

    // Konfigurasi WiFi Access Point (AP Mode)
    WiFi.mode(WIFI_AP);
    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.println("[WIFI] Access Point Aktif:");
    Serial.printf("       SSID     : %s\n", AP_SSID);
    Serial.printf("       Password : %s\n", AP_PASSWORD);
    Serial.printf("       Web URL  : http://%s\n", AP_IP_ADDRESS);

    // DNS Server untuk Captive Portal
    dnsServer.start(53, "*", local_ip);

    // Setup Web Server Routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/data", HTTP_GET, handleApiData);
    server.on("/api/reset_batch", HTTP_POST, handleResetBatch);
    server.on("/api/reset_total", HTTP_POST, handleResetTotal);
    server.on("/api/settings", HTTP_POST, handleSaveSettings);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[HTTP] Web Server aktif dan siap melayani permintaan.");

    lastCalcTime = millis();
}

// ==========================================
// ARDUINO MAIN LOOP
// ==========================================
void loop() {
    // Layani DNS query dan HTTP request
    dnsServer.processNextRequest();
    server.handleClient();

    unsigned long currentMillis = millis();

    // Perhitungan Debit Aliran & Volume setiap 500 ms
    if (currentMillis - lastCalcTime >= 500) {
        unsigned long deltaMs = currentMillis - lastCalcTime;
        lastCalcTime = currentMillis;

        // Ambil jumlah pulsa dalam rentang waktu deltaMs secara aman (atomic)
        noInterrupts();
        uint32_t pulses = isrPulseCount;
        isrPulseCount = 0;
        uint32_t currentBatchPulses = isrBatchPulses;
        interrupts();

        if (pulses > 0) {
            lastPulseSeenMillis = currentMillis;
            isDispensing = true;
            digitalWrite(LED_FLOW_PIN, HIGH);
        } else {
            // Jika lebih dari 1.2 detik tidak ada pulsa, cairan berhenti mengalir (standby)
            if (currentMillis - lastPulseSeenMillis > 1200) {
                isDispensing = false;
                digitalWrite(LED_FLOW_PIN, LOW);
            }
        }

        // Hitung debit aliran (Flow Rate dalam Satuan/menit)
        if (isDispensing && kFactor > 0.0f) {
            float volumeThisPeriod = (float)pulses / kFactor;
            currentFlowRateLpm = (volumeThisPeriod * 60000.0f) / (float)deltaMs;
            
            // Tambahkan ke total akumulasi
            totalLiters += (double)volumeThisPeriod;
        } else {
            currentFlowRateLpm = 0.0f;
        }

        // Hitung volume batch pengisian saat ini
        if (kFactor > 0.0f) {
            batchLiters = (float)currentBatchPulses / kFactor;
            batchCost = batchLiters * pricePerLiter;
        }
    }

    // Auto-save total akumulasi ke NVS saat cairan berhenti mengalir atau per 10 detik
    if (currentMillis - lastNvsSaveMillis >= NVS_AUTO_SAVE_INTERVAL) {
        lastNvsSaveMillis = currentMillis;
        if (!isDispensing) {
            saveTotalToNVS();
        }
    }
}
