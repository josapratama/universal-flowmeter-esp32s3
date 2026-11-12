#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "history.h"
#include "web_page.h"

// ==========================================
// GLOBALS & OBJECTS
// ==========================================
WebServer server(HTTP_PORT);
DNSServer dnsServer;
Preferences preferences;
TransactionHistoryManager historyManager;

// Status & Perhitungan Flow Meter
volatile uint32_t isrPulseCount = 0;
volatile uint32_t isrBatchPulses = 0;
volatile uint32_t lastPulseMicros = 0;

float kFactor = DEFAULT_K_FACTOR;
float pricePerLiter = DEFAULT_PRICE_PER_UNIT;
String fluidName = DEFAULT_FLUID_NAME;
String unitName = DEFAULT_UNIT_NAME;
String currencySymbol = DEFAULT_CURRENCY;
String adminPin = DEFAULT_ADMIN_PIN;

// Inching / Overshoot Compensation (Pencegah over-dispensing)
float overshootLiters = DEFAULT_OVERSHOOT_LITERS;

// Webhook & Cloud Integration (Omni Agent Hub / IoT)
String webhookUrl = "";

// WiFi Station (Home WiFi) Settings
String staSsid = "";
String staPassword = "";
bool staConnected = false;

// Realtime Metrics
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
// SAFETY ALERTS (Dry-Run & Leakage)
// ==========================================
bool dryRunAlarm = false;
bool leakageAlert = false;
uint32_t idlePulseAccumulator = 0;
unsigned long lastAlarmBeepMillis = 0;

// ==========================================
// AUTO-CUTOFF SOLENOID VALVE CONTROL
// ==========================================
enum DispenseMode {
    MODE_MANUAL = 0,
    MODE_PRESET_VOLUME = 1,
    MODE_PRESET_AMOUNT = 2
};

bool valveOpen = false;
DispenseMode activeDispenseMode = MODE_MANUAL;
float targetDispenseVolume = 0.0f;
float targetDispenseAmount = 0.0f;
unsigned long dispenseStartTime = 0;

// Helper: Buzzer Beep
void beepBuzzer(uint16_t durationMs, uint8_t count = 1) {
    for (uint8_t i = 0; i < count; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(durationMs);
        digitalWrite(BUZZER_PIN, LOW);
        if (count > 1 && i < count - 1) delay(80);
    }
}

// Background Task: Kirim Webhook ke Omni Agent Hub / Cloud
void sendWebhookTask(void* pvParameters) {
    String* payload = (String*)pvParameters;
    if (payload && webhookUrl.length() > 5) {
        HTTPClient http;
        http.begin(webhookUrl);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(4000);
        int code = http.POST(*payload);
        Serial.printf("[WEBHOOK] Push ke %s: HTTP %d\n", webhookUrl.c_str(), code);
        http.end();
    }
    delete payload;
    vTaskDelete(NULL);
}

// Helper: Ambil waktu kalender terformat
String getFormattedCurrentTime() {
    time_t now = time(nullptr);
    if (now > 1700000000) {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S WIB", &timeinfo);
        return String(buf);
    }
    uint32_t sec = (uint32_t)(millis() / 1000);
    char buf[32];
    snprintf(buf, sizeof(buf), "Uptime %02u:%02u:%02u", 
             (unsigned int)(sec / 3600), (unsigned int)((sec % 3600) / 60), (unsigned int)(sec % 60));
    return String(buf);
}

// ==========================================
// INTERRUPT SERVICE ROUTINE (AICHI SENSOR)
// ==========================================
void IRAM_ATTR onFlowSensorPulse() {
    uint32_t now = micros();
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
    adminPin = preferences.getString("admin_pin", DEFAULT_ADMIN_PIN);
    overshootLiters = preferences.getFloat("overshoot", DEFAULT_OVERSHOOT_LITERS);
    webhookUrl = preferences.getString("webhook", "");
    staSsid = preferences.getString("sta_ssid", "");
    staPassword = preferences.getString("sta_pass", "");
    preferences.end();

    lastSavedTotalLiters = totalLiters;
    Serial.printf("[NVS] Data dimuat: Cairan=%s, Total=%.2f %s, K=%.3f, Harga=%s %.0f, PIN=%s, Overshoot=%.3f L\n", 
                  fluidName.c_str(), totalLiters, unitName.c_str(), kFactor, currencySymbol.c_str(), 
                  pricePerLiter, adminPin.c_str(), overshootLiters);
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
    preferences.putString("admin_pin", adminPin);
    preferences.putFloat("overshoot", overshootLiters);
    preferences.putString("webhook", webhookUrl);
    preferences.putString("sta_ssid", staSsid);
    preferences.putString("sta_pass", staPassword);
    preferences.end();
    Serial.println("[NVS] Konfigurasi profil cairan dan IoT berhasil disimpan.");
}

// ==========================================
// DISPENSE CONTROL LOGIC
// ==========================================
void startDispense(DispenseMode mode, float target) {
    // Reset alarm sebelum mulai
    dryRunAlarm = false;
    leakageAlert = false;
    idlePulseAccumulator = 0;

    // Reset batch pulses for clean start
    noInterrupts();
    isrBatchPulses = 0;
    interrupts();
    batchLiters = 0.0f;
    batchCost = 0.0f;

    activeDispenseMode = mode;
    if (mode == MODE_PRESET_VOLUME) {
        targetDispenseVolume = target;
        targetDispenseAmount = target * pricePerLiter;
    } else if (mode == MODE_PRESET_AMOUNT) {
        targetDispenseAmount = target;
        targetDispenseVolume = (pricePerLiter > 0.0f) ? (target / pricePerLiter) : 0.0f;
    } else {
        targetDispenseVolume = 0.0f;
        targetDispenseAmount = 0.0f;
    }

    valveOpen = true;
    digitalWrite(RELAY_VALVE_PIN, HIGH);
    dispenseStartTime = millis();
    beepBuzzer(100, 1);

    Serial.printf("[DISPENSE] Mulai pengisian! Mode=%d, TargetVol=%.2f %s, TargetAmt=%s %.0f, Kompensasi=%.3f L\n",
                  mode, targetDispenseVolume, unitName.c_str(), currencySymbol.c_str(), targetDispenseAmount, overshootLiters);
}

void stopDispense(bool autoCutoff = false, const char* stopReason = "normal") {
    if (valveOpen) {
        valveOpen = false;
        digitalWrite(RELAY_VALVE_PIN, LOW);

        const char* modeStr = "manual";
        if (activeDispenseMode == MODE_PRESET_VOLUME) modeStr = "preset_volume";
        else if (activeDispenseMode == MODE_PRESET_AMOUNT) modeStr = "preset_amount";

        // Record to History Log
        historyManager.addTransaction(
            fluidName.c_str(),
            batchLiters,
            unitName.c_str(),
            batchCost,
            currencySymbol.c_str(),
            modeStr
        );

        // Async Webhook trigger ke Omni Agent Hub / VPS
        if (webhookUrl.length() > 5 && staConnected) {
            JsonDocument wDoc;
            wDoc["event"] = autoCutoff ? "DISPENSE_AUTO_CUTOFF" : "DISPENSE_STOPPED";
            wDoc["reason"] = stopReason;
            wDoc["fluid"] = fluidName;
            wDoc["volume"] = batchLiters;
            wDoc["unit"] = unitName;
            wDoc["cost"] = batchCost;
            wDoc["currency"] = currencySymbol;
            wDoc["price_per_unit"] = pricePerLiter;
            wDoc["mode"] = modeStr;
            wDoc["total_odometer"] = totalLiters;
            wDoc["timestamp_epoch"] = (uint32_t)time(nullptr);
            wDoc["time_str"] = getFormattedCurrentTime();

            String* payloadStr = new String();
            serializeJson(wDoc, *payloadStr);
            xTaskCreate(sendWebhookTask, "WebhookTask", 4096, (void*)payloadStr, 1, NULL);
        }

        if (autoCutoff) {
            Serial.println("[DISPENSE] TARGET TERCAPAI! Auto Cut-off Solenoid Valve aktif.");
            beepBuzzer(150, 3); // 3 beeps on auto-cutoff
        } else if (strcmp(stopReason, "dry_run") == 0) {
            Serial.println("[DISPENSE] DRY RUN ALARM! Pompa kosong / tidak ada aliran.");
            beepBuzzer(500, 2);
        } else {
            Serial.println("[DISPENSE] Pengisian dihentikan manual / emergency stop.");
            beepBuzzer(300, 1);
        }
    }
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
    doc["valve_open"] = valveOpen;
    doc["dispense_mode"] = (int)activeDispenseMode;
    doc["target_volume"] = targetDispenseVolume;
    doc["target_amount"] = targetDispenseAmount;
    doc["overshoot_liters"] = overshootLiters;
    doc["history_count"] = historyManager.getCount();
    doc["sta_connected"] = staConnected;
    doc["sta_ip"] = staConnected ? WiFi.localIP().toString() : "";
    doc["ap_ip"] = WiFi.softAPIP().toString();
    doc["system_time"] = getFormattedCurrentTime();
    doc["system_epoch"] = (uint32_t)time(nullptr);
    doc["dry_run_alarm"] = dryRunAlarm;
    doc["leakage_alert"] = leakageAlert;
    doc["webhook_url"] = webhookUrl;

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void handleDispenseStart() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }

    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));

    String modeStr = doc["mode"] | "manual";
    float target = doc["target"] | 0.0f;

    DispenseMode mode = MODE_MANUAL;
    if (modeStr == "volume") mode = MODE_PRESET_VOLUME;
    else if (modeStr == "amount") mode = MODE_PRESET_AMOUNT;

    startDispense(mode, target);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Pengisian dimulai\"}");
}

void handleDispenseStop() {
    stopDispense(false, "manual");
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Pengisian dihentikan\"}");
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
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"PIN diperlukan\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String pin = doc["pin"] | "";

    if (pin != adminPin) {
        server.send(403, "application/json", "{\"error\":\"PIN Admin salah!\"}");
        return;
    }

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

    beepBuzzer(400, 2);
    Serial.println("[FLOW] Total akumulasi seumur hidup berhasil dinolkan!");
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Total seumur hidup berhasil dinolkan\"}");
}

void handleSaveSettings() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }

    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String pin = doc["pin"] | "";

    if (pin != adminPin) {
        server.send(403, "application/json", "{\"error\":\"PIN Admin salah!\"}");
        return;
    }

    if (doc["fluid_name"].is<String>()) fluidName = doc["fluid_name"].as<String>();
    if (doc["unit_name"].is<String>()) unitName = doc["unit_name"].as<String>();
    if (doc["currency"].is<String>()) currencySymbol = doc["currency"].as<String>();
    if (doc["price"].is<float>()) pricePerLiter = doc["price"].as<float>();
    if (doc["k_factor"].is<float>()) {
        float newK = doc["k_factor"].as<float>();
        if (newK > 0.0f) kFactor = newK;
    }
    if (doc["overshoot"].is<float>()) {
        float ov = doc["overshoot"].as<float>();
        if (ov >= 0.0f && ov <= 2.0f) overshootLiters = ov;
    }
    if (doc["webhook"].is<String>()) {
        webhookUrl = doc["webhook"].as<String>();
    }

    saveConfigToNVS();
    beepBuzzer(150, 1);
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Pengaturan tersimpan ke ESP32\"}");
}

// Wizard Kalibrasi Bejana Ukur (10L / 20L)
void handleCalibrateBejana() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }

    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String pin = doc["pin"] | "";

    if (pin != adminPin) {
        server.send(403, "application/json", "{\"error\":\"PIN Admin salah!\"}");
        return;
    }

    float targetVolume = doc["target_volume"] | 10.0f;
    uint32_t pulses = 0;
    if (doc["actual_pulses"].is<uint32_t>()) {
        pulses = doc["actual_pulses"].as<uint32_t>();
    } else {
        // Ambil dari batch saat ini
        noInterrupts();
        pulses = isrBatchPulses;
        interrupts();
    }

    if (targetVolume <= 0.1f || pulses == 0) {
        server.send(400, "application/json", "{\"error\":\"Pulsa atau volume tidak valid! Alirkan cairan ke bejana terlebih dahulu.\"}");
        return;
    }

    float newK = (float)pulses / targetVolume;
    kFactor = newK;
    saveConfigToNVS();

    beepBuzzer(200, 2);
    Serial.printf("[CALIBRATION] Kalibrasi Bejana Selesai! Vol=%.2f L, Pulsa=%u, K-Factor Baru=%.3f pulsa/L\n", 
                  targetVolume, (unsigned int)pulses, newK);

    JsonDocument resp;
    resp["status"] = "ok";
    resp["k_factor"] = newK;
    resp["pulses"] = pulses;
    resp["target_volume"] = targetVolume;
    resp["message"] = "Kalibrasi bejana sukses! K-Factor baru tersimpan.";
    String jsonResp;
    serializeJson(resp, jsonResp);
    server.send(200, "application/json", jsonResp);
}

// Sinkronisasi Waktu dari Browser Smartphone (Fallback Offline)
void handleTimeSync() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    uint32_t epoch = doc["epoch"] | 0;

    if (epoch > 1700000000) {
        struct timeval tv = { (time_t)epoch, 0 };
        settimeofday(&tv, NULL);
        Serial.printf("[TIME] Jam RTC berhasil disinkronkan dari browser: %s\n", getFormattedCurrentTime().c_str());
        server.send(200, "application/json", "{\"status\":\"ok\",\"time\": \"" + getFormattedCurrentTime() + "\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Timestamp epoch tidak valid\"}");
    }
}

// Clear Safety Alarms
void handleClearAlerts() {
    dryRunAlarm = false;
    leakageAlert = false;
    idlePulseAccumulator = 0;
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Alarm keamanan direset\"}");
}

void handleAdminVerify() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"PIN diperlukan\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String pin = doc["pin"] | "";

    if (pin == adminPin) {
        server.send(200, "application/json", "{\"status\":\"ok\",\"verified\":true}");
    } else {
        server.send(403, "application/json", "{\"status\":\"error\",\"verified\":false,\"error\":\"PIN salah\"}");
    }
}

void handleAdminSetPin() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String oldPin = doc["old_pin"] | "";
    String newPin = doc["new_pin"] | "";

    if (oldPin != adminPin) {
        server.send(403, "application/json", "{\"error\":\"PIN Lama salah!\"}");
        return;
    }
    if (newPin.length() < 4) {
        server.send(400, "application/json", "{\"error\":\"PIN Baru minimal 4 karakter!\"}");
        return;
    }

    adminPin = newPin;
    saveConfigToNVS();
    beepBuzzer(200, 2);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"PIN Admin berhasil diubah\"}");
}

void handleGetHistory() {
    server.send(200, "application/json", historyManager.toJson());
}

void handleExportHistoryCsv() {
    server.sendHeader("Content-Disposition", "attachment; filename=\"riwayat_transaksi_flowmeter.csv\"");
    server.send(200, "text/csv", historyManager.toCsv());
}

void handleClearHistory() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"PIN diperlukan\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String pin = doc["pin"] | "";

    if (pin != adminPin) {
        server.send(403, "application/json", "{\"error\":\"PIN Admin salah!\"}");
        return;
    }

    historyManager.clear();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Riwayat transaksi dikosongkan\"}");
}

void handleWifiSettings() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String pin = doc["pin"] | "";

    if (pin != adminPin) {
        server.send(403, "application/json", "{\"error\":\"PIN Admin salah!\"}");
        return;
    }

    staSsid = doc["ssid"] | "";
    staPassword = doc["password"] | "";
    saveConfigToNVS();

    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"WiFi tersimpan, mencoba menghubungkan...\"}");

    if (staSsid.length() > 0) {
        WiFi.begin(staSsid.c_str(), staPassword.c_str());
    }
}

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
    Serial.println("  UNIVERSAL FLOW METER ESP32-S3 | SMART NVR");
    Serial.println("==============================================");

    // Setup GPIO
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(LED_FLOW_PIN, OUTPUT);
    digitalWrite(LED_FLOW_PIN, LOW);

    pinMode(RELAY_VALVE_PIN, OUTPUT);
    digitalWrite(RELAY_VALVE_PIN, LOW); // Solenoid valve initially closed

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Load NVS Settings & History
    loadSettingsFromNVS();
    historyManager.begin();

    // Attach hardware interrupt ke sensor AICHI
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), onFlowSensorPulse, FALLING);
    Serial.printf("[SENSOR] Interrupt terpasang pada GPIO %d (FALLING edge)\n", SENSOR_PIN);

    // Konfigurasi WiFi DUAL MODE (AP + STA)
    WiFi.mode(WIFI_AP_STA);

    // 1. Setup Access Point
    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.println("[WIFI] Access Point Aktif:");
    Serial.printf("       SSID     : %s\n", AP_SSID);
    Serial.printf("       Password : %s\n", AP_PASSWORD);
    Serial.printf("       Web URL  : http://%s\n", AP_IP_ADDRESS);

    // 2. Setup Station mode jika SSID tersimpan
    if (staSsid.length() > 0) {
        Serial.printf("[WIFI] Mencoba konek ke WiFi Rumah: %s\n", staSsid.c_str());
        WiFi.begin(staSsid.c_str(), staPassword.c_str());
    }

    // 3. Setup SNTP Time Synchronization (GMT+7 WIB)
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    // DNS Server untuk Captive Portal
    dnsServer.start(53, "*", local_ip);

    // Web Server Endpoints
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/data", HTTP_GET, handleApiData);
    server.on("/api/dispense/start", HTTP_POST, handleDispenseStart);
    server.on("/api/dispense/stop", HTTP_POST, handleDispenseStop);
    server.on("/api/reset_batch", HTTP_POST, handleResetBatch);
    server.on("/api/reset_total", HTTP_POST, handleResetTotal);
    server.on("/api/settings", HTTP_POST, handleSaveSettings);
    server.on("/api/calibrate/bejana", HTTP_POST, handleCalibrateBejana);
    server.on("/api/time/sync", HTTP_POST, handleTimeSync);
    server.on("/api/alerts/clear", HTTP_POST, handleClearAlerts);
    server.on("/api/admin/verify", HTTP_POST, handleAdminVerify);
    server.on("/api/admin/set_pin", HTTP_POST, handleAdminSetPin);
    server.on("/api/history", HTTP_GET, handleGetHistory);
    server.on("/api/history/export", HTTP_GET, handleExportHistoryCsv);
    server.on("/api/history/clear", HTTP_POST, handleClearHistory);
    server.on("/api/wifi/settings", HTTP_POST, handleWifiSettings);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[HTTP] Web Server aktif dan siap melayani permintaan.");

    beepBuzzer(80, 2); // Startup sound
    lastCalcTime = millis();
}

// ==========================================
// ARDUINO MAIN LOOP
// ==========================================
void loop() {
    dnsServer.processNextRequest();
    server.handleClient();

    unsigned long currentMillis = millis();

    // Check STA connection status
    staConnected = (WiFi.status() == WL_CONNECTED);

    // Perhitungan Debit & Volume setiap 300 ms (fast response)
    if (currentMillis - lastCalcTime >= 300) {
        unsigned long deltaMs = currentMillis - lastCalcTime;
        lastCalcTime = currentMillis;

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
            if (currentMillis - lastPulseSeenMillis > 1200) {
                isDispensing = false;
                digitalWrite(LED_FLOW_PIN, LOW);
            }
        }

        // Hitung debit aliran (Flow Rate Satuan / Menit)
        if (isDispensing && kFactor > 0.0f) {
            float volumeThisPeriod = (float)pulses / kFactor;
            currentFlowRateLpm = (volumeThisPeriod * 60000.0f) / (float)deltaMs;
            totalLiters += (double)volumeThisPeriod;
        } else {
            currentFlowRateLpm = 0.0f;
        }

        // Hitung volume batch
        if (kFactor > 0.0f) {
            batchLiters = (float)currentBatchPulses / kFactor;
            batchCost = batchLiters * pricePerLiter;
        }

        // ==========================================
        // AUTO CUT-OFF EVALUATION (DENGAN OVERSHOOT COMPENSATION)
        // ==========================================
        if (valveOpen) {
            // Hitung target efektif dengan kompensasi sisa inersia selang
            float effectiveCutoffVolume = targetDispenseVolume > overshootLiters ? (targetDispenseVolume - overshootLiters) : targetDispenseVolume;
            float effectiveCutoffCost = targetDispenseAmount > (overshootLiters * pricePerLiter) ? (targetDispenseAmount - (overshootLiters * pricePerLiter)) : targetDispenseAmount;

            if (activeDispenseMode == MODE_PRESET_VOLUME && targetDispenseVolume > 0.0f) {
                if (batchLiters >= effectiveCutoffVolume) {
                    stopDispense(true, "auto_cutoff"); // Auto cut-off triggered!
                }
            } else if (activeDispenseMode == MODE_PRESET_AMOUNT && targetDispenseAmount > 0.0f) {
                if (batchCost >= effectiveCutoffCost) {
                    stopDispense(true, "auto_cutoff"); // Auto cut-off triggered!
                }
            }

            // SAFETY: Dry-Run Protection (Jika valve buka tapi tidak ada cairan > 6 detik)
            if (currentMillis - dispenseStartTime >= DRY_RUN_TIMEOUT_MS) {
                if (currentFlowRateLpm < 0.15f) {
                    dryRunAlarm = true;
                    stopDispense(false, "dry_run");
                }
            }
        } else {
            // SAFETY: Deteksi Kebocoran Pipa (Jika valve tutup tapi pulsa terus masuk)
            if (pulses > 0) {
                idlePulseAccumulator += pulses;
                if (idlePulseAccumulator >= LEAKAGE_THRESHOLD_PULSES) {
                    leakageAlert = true;
                }
            }
        }
    }

    // Safety Alarm Buzzer Beep
    if (dryRunAlarm || leakageAlert) {
        if (currentMillis - lastAlarmBeepMillis >= 1500) {
            lastAlarmBeepMillis = currentMillis;
            digitalWrite(BUZZER_PIN, HIGH);
            delay(40);
            digitalWrite(BUZZER_PIN, LOW);
        }
    }

    // Auto-save total akumulasi ke NVS per 10 detik saat tidak ada aliran
    if (currentMillis - lastNvsSaveMillis >= NVS_AUTO_SAVE_INTERVAL) {
        lastNvsSaveMillis = currentMillis;
        if (!isDispensing && !valveOpen) {
            saveTotalToNVS();
        }
    }
}
