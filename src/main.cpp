#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Update.h>
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "history.h"
#include "web_page.h"
#include "lcd_display.h"

// ==========================================
// NON-BLOCKING BUZZER STATE MACHINE
// ==========================================
class NonBlockingBuzzer {
private:
    uint8_t pin;
    uint16_t onDuration;
    uint16_t offDuration;
    uint8_t totalBeeps;
    uint8_t currentBeep;
    bool active;
    unsigned long lastTransition;

public:
    explicit NonBlockingBuzzer(uint8_t p)
        : pin(p), onDuration(0), offDuration(0), totalBeeps(0), currentBeep(0), active(false), lastTransition(0) {}

    void begin() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    void beep(uint16_t durationMs, uint8_t count = 1, uint16_t pauseMs = 80) {
        if (count == 0 || durationMs == 0) return;
        onDuration = durationMs;
        offDuration = pauseMs;
        totalBeeps = count;
        currentBeep = 0;
        active = true;
        digitalWrite(pin, HIGH);
        lastTransition = millis();
    }

    void stop() {
        totalBeeps = 0;
        currentBeep = 0;
        active = false;
        digitalWrite(pin, LOW);
    }

    void update(unsigned long now) {
        if (totalBeeps == 0) return;

        if (active) {
            if (now - lastTransition >= onDuration) {
                digitalWrite(pin, LOW);
                active = false;
                lastTransition = now;
                currentBeep++;
                if (currentBeep >= totalBeeps) {
                    totalBeeps = 0;
                }
            }
        } else {
            if (now - lastTransition >= offDuration) {
                digitalWrite(pin, HIGH);
                active = true;
                lastTransition = now;
            }
        }
    }
};

// ==========================================
// GLOBALS & OBJECTS
// ==========================================
WebServer server(HTTP_PORT);
DNSServer dnsServer;
Preferences preferences;
TransactionHistoryManager historyManager;
FlowMeterLcd flowLcd;
NonBlockingBuzzer buzzer(BUZZER_PIN);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

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
String activeFluidId = "biosolar";

// Multi-Point Flow Calibration
bool multiPointCalibEnabled = true;
float kLow = DEFAULT_K_LOW;
float kMid = DEFAULT_K_MID;
float kHigh = DEFAULT_K_HIGH;

// Rekap Shift Operator & Akumulasi Harian
float shiftLiters = 0.0f;
float shiftCost = 0.0f;
uint32_t shiftTxCount = 0;

float todayLiters = 0.0f;
float todayCost = 0.0f;
uint32_t todayTxCount = 0;
String todayDateStr = "";

// Transaksi Terakhir (untuk 1-Click Cetak Struk)
float lastBatchLiters = 0.0f;
float lastBatchCost = 0.0f;
String lastBatchTimeStr = "-";
uint32_t lastBatchId = 0;
String lastBatchMode = "manual";

// Inching / Overshoot Compensation (Pencegah over-dispensing)
float overshootLiters = DEFAULT_OVERSHOOT_LITERS;

// Two-Stage Valve Control (Fast Flow + Slow Inching Flow)
bool twoStageEnabled = DEFAULT_TWO_STAGE_ENABLED;
float slowStageLiters = DEFAULT_SLOW_STAGE_LITERS;
bool valveFastOpen = false;
bool valveSlowOpen = false;
bool valveOpen = false;

// Smart Tank Inventory Management
float tankCapacityLiters = DEFAULT_TANK_CAPACITY_L;
float tankRemainingLiters = DEFAULT_TANK_REMAINING_L;
float tankLowAlertLiters = DEFAULT_TANK_LOW_ALERT_L;
bool tankLowAlarm = false;

// ESC/POS Thermal Receipt Store Info
String storeName = DEFAULT_STORE_NAME;
String storeAddress = DEFAULT_STORE_ADDRESS;
String storeFooter = DEFAULT_STORE_FOOTER;

// Webhook & Cloud Integration (Omni Agent Hub / IoT)
String webhookUrl = "";

// WiFi Station (Home WiFi) Settings
String staSsid = "";
String staPassword = "";
bool staConnected = false;

// MQTT Telemetry & Home Assistant Auto-Discovery
bool mqttEnabled = DEFAULT_MQTT_ENABLED;
String mqttBroker = DEFAULT_MQTT_BROKER;
uint16_t mqttPort = DEFAULT_MQTT_PORT;
String mqttUser = DEFAULT_MQTT_USER;
String mqttPass = DEFAULT_MQTT_PASS;
String mqttTopic = DEFAULT_MQTT_TOPIC;
String mqttClientId = DEFAULT_MQTT_CLIENT_ID;
unsigned long lastMqttReconnect = 0;
unsigned long lastMqttPublish = 0;
bool mqttHaDiscoverySent = false;

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
bool nvsDirty = false;

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

DispenseMode activeDispenseMode = MODE_MANUAL;
float targetDispenseVolume = 0.0f;
float targetDispenseAmount = 0.0f;
unsigned long dispenseStartTime = 0;

// Helper: Non-blocking Buzzer Beep
inline void beepBuzzer(uint16_t durationMs, uint8_t count = 1, uint16_t pauseMs = 80) {
    buzzer.beep(durationMs, count, pauseMs);
}

// Helper: Two-Stage Valve Control
void setValves(bool fast, bool slow) {
    valveFastOpen = fast;
    valveSlowOpen = slow;
    valveOpen = (fast || slow);
    digitalWrite(RELAY_VALVE_PIN, fast ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
    digitalWrite(RELAY_VALVE_SLOW_PIN, slow ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
}

inline void setValve(bool open) {
    if (open) {
        setValves(true, true); // Buka kedua tahap katup saat pengisian normal dimulai
    } else {
        setValves(false, false);
    }
}

inline void setFlowLed(bool on) {
    digitalWrite(LED_FLOW_PIN, on ? LED_ACTIVE_LEVEL : !LED_ACTIVE_LEVEL);
}

// Background Task: Kirim Webhook ke Omni Agent Hub / Cloud (Stack 8192 bytes aman untuk TLS)
void sendWebhookTask(void* pvParameters) {
    String* payload = (String*)pvParameters;
    if (payload && webhookUrl.length() > 5) {
        HTTPClient http;
        http.begin(webhookUrl);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(5000);
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

// Helper: Ambil string tanggal hari ini (YYYY-MM-DD)
String getCurrentDateString() {
    time_t now = time(nullptr);
    if (now > 1700000000) {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        char buf[16];
        strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
        return String(buf);
    }
    return "OFFLINE";
}

// Helper: Kalibrasi Aliran Multi-Titik Dinamis berdasarkan Debit LPM
float getEffectiveKFactor(float lpm) {
    if (!multiPointCalibEnabled) return kFactor;
    if (lpm <= CALIB_LOW_LPM_LIMIT) {
        return kLow;
    } else if (lpm >= CALIB_HIGH_LPM_LIMIT) {
        return kHigh;
    } else {
        float factor = (lpm - CALIB_LOW_LPM_LIMIT) / (CALIB_HIGH_LPM_LIMIT - CALIB_LOW_LPM_LIMIT);
        return kLow + factor * (kHigh - kLow);
    }
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
    activeFluidId = preferences.getString("fluid_id", "biosolar");
    overshootLiters = preferences.getFloat("overshoot", DEFAULT_OVERSHOOT_LITERS);
    webhookUrl = preferences.getString("webhook", "");
    staSsid = preferences.getString("sta_ssid", "");
    staPassword = preferences.getString("sta_pass", "");

    // Load Rekap Shift & Daily
    shiftLiters = preferences.getFloat("s_liters", 0.0f);
    shiftCost = preferences.getFloat("s_cost", 0.0f);
    shiftTxCount = preferences.getUInt("s_count", 0);
    todayLiters = preferences.getFloat("t_liters", 0.0f);
    todayCost = preferences.getFloat("t_cost", 0.0f);
    todayTxCount = preferences.getUInt("t_count", 0);
    todayDateStr = preferences.getString("t_date", "");

    // Load Multi-Point Calibration
    multiPointCalibEnabled = preferences.getBool("mp_en", true);
    kLow = preferences.getFloat("k_low", DEFAULT_K_LOW);
    kMid = preferences.getFloat("k_mid", DEFAULT_K_MID);
    kHigh = preferences.getFloat("k_high", DEFAULT_K_HIGH);

    // Load Two-Stage Valve Settings
    twoStageEnabled = preferences.getBool("two_stage_en", DEFAULT_TWO_STAGE_ENABLED);
    slowStageLiters = preferences.getFloat("slow_liters", DEFAULT_SLOW_STAGE_LITERS);

    // Load Smart Tank Inventory
    tankCapacityLiters = preferences.getFloat("tank_cap", DEFAULT_TANK_CAPACITY_L);
    tankRemainingLiters = preferences.getFloat("tank_rem", DEFAULT_TANK_REMAINING_L);
    tankLowAlertLiters = preferences.getFloat("tank_alert", DEFAULT_TANK_LOW_ALERT_L);
    tankLowAlarm = (tankRemainingLiters <= tankLowAlertLiters);

    // Load ESC/POS Printer Store Info
    storeName = preferences.getString("store_name", DEFAULT_STORE_NAME);
    storeAddress = preferences.getString("store_addr", DEFAULT_STORE_ADDRESS);
    storeFooter = preferences.getString("store_foot", DEFAULT_STORE_FOOTER);

    // Load MQTT Settings
    mqttEnabled = preferences.getBool("mqtt_en", DEFAULT_MQTT_ENABLED);
    mqttBroker = preferences.getString("mqtt_host", DEFAULT_MQTT_BROKER);
    mqttPort = preferences.getUShort("mqtt_port", DEFAULT_MQTT_PORT);
    mqttUser = preferences.getString("mqtt_user", DEFAULT_MQTT_USER);
    mqttPass = preferences.getString("mqtt_pass", DEFAULT_MQTT_PASS);
    mqttTopic = preferences.getString("mqtt_topic", DEFAULT_MQTT_TOPIC);

    preferences.end();

    lastSavedTotalLiters = totalLiters;
    nvsDirty = false;

    Serial.printf("[NVS] Data dimuat: Cairan=%s, Total=%.2f %s, K=%.3f, Harga=%s %.0f, Tangki=%.1f/%.1f L\n", 
                  fluidName.c_str(), totalLiters, unitName.c_str(), kFactor, currencySymbol.c_str(), 
                  pricePerLiter, tankRemainingLiters, tankCapacityLiters);
}

void saveTotalToNVS() {
    if (nvsDirty || abs(totalLiters - lastSavedTotalLiters) > 0.01) {
        preferences.begin("flowmeter", false);
        preferences.putDouble("total_l", totalLiters);
        preferences.putFloat("s_liters", shiftLiters);
        preferences.putFloat("s_cost", shiftCost);
        preferences.putUInt("s_count", shiftTxCount);
        preferences.putFloat("t_liters", todayLiters);
        preferences.putFloat("t_cost", todayCost);
        preferences.putUInt("t_count", todayTxCount);
        preferences.putString("t_date", todayDateStr);
        preferences.putFloat("tank_rem", tankRemainingLiters);
        preferences.end();

        lastSavedTotalLiters = totalLiters;
        nvsDirty = false;
        Serial.printf("[NVS] Total & Tangki tersimpan: %.3f %s (Sisa Tangki: %.1f L)\n", totalLiters, unitName.c_str(), tankRemainingLiters);
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
    preferences.putString("fluid_id", activeFluidId);
    preferences.putFloat("overshoot", overshootLiters);
    preferences.putString("webhook", webhookUrl);
    preferences.putString("sta_ssid", staSsid);
    preferences.putString("sta_pass", staPassword);
    preferences.putBool("mp_en", multiPointCalibEnabled);
    preferences.putFloat("k_low", kLow);
    preferences.putFloat("k_mid", kMid);
    preferences.putFloat("k_high", kHigh);
    preferences.putBool("two_stage_en", twoStageEnabled);
    preferences.putFloat("slow_liters", slowStageLiters);
    preferences.putFloat("tank_cap", tankCapacityLiters);
    preferences.putFloat("tank_rem", tankRemainingLiters);
    preferences.putFloat("tank_alert", tankLowAlertLiters);
    preferences.putString("store_name", storeName);
    preferences.putString("store_addr", storeAddress);
    preferences.putString("store_foot", storeFooter);
    preferences.putBool("mqtt_en", mqttEnabled);
    preferences.putString("mqtt_host", mqttBroker);
    preferences.putUShort("mqtt_port", mqttPort);
    preferences.putString("mqtt_user", mqttUser);
    preferences.putString("mqtt_pass", mqttPass);
    preferences.putString("mqtt_topic", mqttTopic);
    preferences.end();

    nvsDirty = false;
    Serial.println("[NVS] Konfigurasi sistem tersimpan.");
}

// ==========================================
// MQTT & HOME ASSISTANT DISCOVERY
// ==========================================
void setupMqtt() {
    if (mqttBroker.length() > 0) {
        mqttClient.setServer(mqttBroker.c_str(), mqttPort);
        mqttClient.setBufferSize(1024);
    }
}

void publishHaDiscovery() {
    if (!mqttClient.connected()) return;

    const char* devId = "flowmeter_aichi_esp32";
    const char* devName = "Smart AICHI Flowmeter";
    const char* devModel = "Universal Flowmeter S3/C3";
    const char* devMfr = "OpenSource Hardware";

    // Sensor 1: Debit Aliran (Flow Rate)
    {
        JsonDocument doc;
        doc["name"] = "Debit Aliran";
        doc["stat_t"] = mqttTopic;
        doc["unit_of_meas"] = "L/min";
        doc["val_tpl"] = "{{ value_json.flow_rate_lpm }}";
        doc["uniq_id"] = "flowmeter_flow_rate";
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["ids"] = devId;
        dev["name"] = devName;
        dev["mdl"] = devModel;
        dev["mf"] = devMfr;
        String p; serializeJson(doc, p);
        mqttClient.publish("homeassistant/sensor/flowmeter/flow_rate/config", p.c_str(), true);
    }

    // Sensor 2: Volume Batch Pengisian
    {
        JsonDocument doc;
        doc["name"] = "Volume Pengisian";
        doc["stat_t"] = mqttTopic;
        doc["unit_of_meas"] = "L";
        doc["val_tpl"] = "{{ value_json.batch_liters }}";
        doc["uniq_id"] = "flowmeter_batch_volume";
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["ids"] = devId;
        String p; serializeJson(doc, p);
        mqttClient.publish("homeassistant/sensor/flowmeter/batch_volume/config", p.c_str(), true);
    }

    // Sensor 3: Total Akumulasi Odometer
    {
        JsonDocument doc;
        doc["name"] = "Total Akumulasi";
        doc["stat_t"] = mqttTopic;
        doc["unit_of_meas"] = "L";
        doc["stat_cla"] = "total_increasing";
        doc["val_tpl"] = "{{ value_json.total_liters }}";
        doc["uniq_id"] = "flowmeter_total_volume";
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["ids"] = devId;
        String p; serializeJson(doc, p);
        mqttClient.publish("homeassistant/sensor/flowmeter/total_volume/config", p.c_str(), true);
    }

    // Sensor 4: Sisa Stok Tangki
    {
        JsonDocument doc;
        doc["name"] = "Sisa Stok Tangki";
        doc["stat_t"] = mqttTopic;
        doc["unit_of_meas"] = "L";
        doc["val_tpl"] = "{{ value_json.tank_remaining_liters }}";
        doc["uniq_id"] = "flowmeter_tank_remaining";
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["ids"] = devId;
        String p; serializeJson(doc, p);
        mqttClient.publish("homeassistant/sensor/flowmeter/tank_remaining/config", p.c_str(), true);
    }

    // Binary Sensor 5: Katup Terbuka (Valve Open)
    {
        JsonDocument doc;
        doc["name"] = "Status Katup Selenoid";
        doc["stat_t"] = mqttTopic;
        doc["val_tpl"] = "{{ 'ON' if value_json.valve_open else 'OFF' }}";
        doc["uniq_id"] = "flowmeter_valve_state";
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["ids"] = devId;
        String p; serializeJson(doc, p);
        mqttClient.publish("homeassistant/binary_sensor/flowmeter/valve/config", p.c_str(), true);
    }

    mqttHaDiscoverySent = true;
    Serial.println("[MQTT] Home Assistant MQTT Auto-Discovery berhasil dipublikasikan.");
}

void publishMqttTelemetry() {
    if (!mqttClient.connected()) return;

    JsonDocument doc;
    doc["flow_rate_lpm"] = currentFlowRateLpm;
    doc["batch_liters"] = batchLiters;
    doc["batch_cost"] = batchCost;
    doc["total_liters"] = totalLiters;
    doc["tank_remaining_liters"] = tankRemainingLiters;
    doc["tank_capacity_liters"] = tankCapacityLiters;
    doc["tank_percentage"] = (tankCapacityLiters > 0.0f) ? ((tankRemainingLiters / tankCapacityLiters) * 100.0f) : 0.0f;
    doc["tank_low"] = tankLowAlarm;
    doc["is_dispensing"] = isDispensing;
    doc["valve_open"] = valveOpen;
    doc["valve_fast_open"] = valveFastOpen;
    doc["valve_slow_open"] = valveSlowOpen;
    doc["fluid"] = fluidName;
    doc["price_per_liter"] = pricePerLiter;
    doc["uptime_sec"] = (uint32_t)(millis() / 1000);

    String payload;
    serializeJson(doc, payload);
    mqttClient.publish(mqttTopic.c_str(), payload.c_str());
}

void handleMqtt(unsigned long now) {
    if (!mqttEnabled || !staConnected || mqttBroker.length() == 0) return;

    if (!mqttClient.connected()) {
        if (now - lastMqttReconnect >= 5000) {
            lastMqttReconnect = now;
            String clientId = mqttClientId + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
            bool connected = false;
            if (mqttUser.length() > 0) {
                connected = mqttClient.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str());
            } else {
                connected = mqttClient.connect(clientId.c_str());
            }

            if (connected) {
                Serial.printf("[MQTT] Terhubung ke Broker %s:%u\n", mqttBroker.c_str(), mqttPort);
                if (!mqttHaDiscoverySent) {
                    publishHaDiscovery();
                }
                publishMqttTelemetry();
            }
        }
    } else {
        mqttClient.loop();
        unsigned long interval = isDispensing ? 1000 : 10000;
        if (now - lastMqttPublish >= interval) {
            lastMqttPublish = now;
            publishMqttTelemetry();
        }
    }
}

// ==========================================
// ESC/POS THERMAL RECEIPT PRINTER GENERATOR
// ==========================================
String generateEscPosReceipt(uint32_t id, const String& timeStr, const String& fluid, float vol, float price, float cost, const String& mode) {
    String esc = "";
    esc += "\x1B\x40";       // Initialize printer
    esc += "\x1B\x61\x01";   // Center align
    esc += "\x1B\x45\x01";   // Bold ON
    esc += storeName + "\n";
    esc += "\x1B\x45\x00";   // Bold OFF
    esc += storeAddress + "\n";
    esc += "--------------------------------\n";
    esc += "STRUK BUKTI PENGISIAN BBM\n";
    esc += "--------------------------------\n";
    esc += "\x1B\x61\x00";   // Left align

    char buf[64];
    snprintf(buf, sizeof(buf), "No. Trx : #%05u\n", (unsigned int)id);
    esc += buf;
    snprintf(buf, sizeof(buf), "Waktu   : %s\n", timeStr.c_str());
    esc += buf;
    snprintf(buf, sizeof(buf), "Produk  : %s\n", fluid.c_str());
    esc += buf;
    snprintf(buf, sizeof(buf), "Harga/L : Rp %.0f\n", price);
    esc += buf;
    snprintf(buf, sizeof(buf), "Mode    : %s\n", mode.c_str());
    esc += buf;
    esc += "--------------------------------\n";

    esc += "\x1B\x45\x01";   // Bold
    snprintf(buf, sizeof(buf), "VOLUME  : %.3f L\n", vol);
    esc += buf;
    snprintf(buf, sizeof(buf), "TOTAL   : Rp %.0f\n", cost);
    esc += buf;
    esc += "\x1B\x45\x00";   // Bold OFF

    esc += "--------------------------------\n";
    esc += "\x1B\x61\x01";   // Center align
    esc += storeFooter + "\n";
    esc += "Simpan struk ini sbg bukti sah\n";
    esc += "\n\n\n";
    esc += "\x1D\x56\x41\x03"; // Cut paper
    return esc;
}

String generateTextReceipt(uint32_t id, const String& timeStr, const String& fluid, float vol, float price, float cost, const String& mode) {
    String txt = "";
    txt += "================================\n";
    txt += "       " + storeName + "\n";
    txt += "    " + storeAddress + "\n";
    txt += "================================\n";
    char buf[64];
    snprintf(buf, sizeof(buf), "No. Trx : #%05u\n", (unsigned int)id);
    txt += buf;
    snprintf(buf, sizeof(buf), "Waktu   : %s\n", timeStr.c_str());
    txt += buf;
    snprintf(buf, sizeof(buf), "Produk  : %s\n", fluid.c_str());
    txt += buf;
    snprintf(buf, sizeof(buf), "Harga/L : Rp %.0f\n", price);
    txt += buf;
    snprintf(buf, sizeof(buf), "Mode    : %s\n", mode.c_str());
    txt += buf;
    txt += "--------------------------------\n";
    snprintf(buf, sizeof(buf), "VOLUME  : %.3f Liter\n", vol);
    txt += buf;
    snprintf(buf, sizeof(buf), "TOTAL   : Rp %.0f\n", cost);
    txt += buf;
    txt += "================================\n";
    txt += "  " + storeFooter + "\n";
    txt += "================================\n";
    return txt;
}

// ==========================================
// DISPENSE CONTROL LOGIC
// ==========================================
void startDispense(DispenseMode mode, float target) {
    dryRunAlarm = false;
    leakageAlert = false;
    idlePulseAccumulator = 0;

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

    setValve(true);
    dispenseStartTime = millis();
    beepBuzzer(100, 1);

    Serial.printf("[DISPENSE] Mulai pengisian! Mode=%d, TargetVol=%.2f %s, TargetAmt=%s %.0f, Kompensasi=%.3f L\n",
                  mode, targetDispenseVolume, unitName.c_str(), currencySymbol.c_str(), targetDispenseAmount, overshootLiters);
}

void stopDispense(bool autoCutoff = false, const char* stopReason = "normal") {
    bool wasDispensing = valveOpen || (batchLiters >= 0.005f);
    if (valveOpen) {
        setValve(false);
    }

    if (wasDispensing && batchLiters >= 0.005f) {
        const char* modeStr = "manual";
        if (activeDispenseMode == MODE_PRESET_VOLUME) modeStr = "preset_volume";
        else if (activeDispenseMode == MODE_PRESET_AMOUNT) modeStr = "preset_amount";

        // Update Rekap Shift Operator
        shiftLiters += batchLiters;
        shiftCost += batchCost;
        shiftTxCount++;

        // Update Akumulasi Harian (Auto reset jika ganti tanggal)
        String todayStr = getCurrentDateString();
        if (todayDateStr != "" && todayStr != "OFFLINE" && todayStr != todayDateStr) {
            todayLiters = 0.0f;
            todayCost = 0.0f;
            todayTxCount = 0;
        }
        if (todayStr != "OFFLINE") {
            todayDateStr = todayStr;
        }
        todayLiters += batchLiters;
        todayCost += batchCost;
        todayTxCount++;

        // Simpan data batch terakhir untuk Cetak Struk Instan
        lastBatchLiters = batchLiters;
        lastBatchCost = batchCost;
        lastBatchTimeStr = getFormattedCurrentTime();
        lastBatchId = historyManager.getCount() + 1;
        lastBatchMode = modeStr;

        // Record to History Log
        historyManager.addTransaction(
            fluidName.c_str(),
            batchLiters,
            unitName.c_str(),
            batchCost,
            currencySymbol.c_str(),
            modeStr
        );

        // Async Webhook trigger ke Omni Agent Hub / VPS (Stack 8192)
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
            wDoc["tank_remaining"] = tankRemainingLiters;
            wDoc["timestamp_epoch"] = (uint32_t)time(nullptr);
            wDoc["time_str"] = getFormattedCurrentTime();

            String* payloadStr = new String();
            serializeJson(wDoc, *payloadStr);
            xTaskCreate(sendWebhookTask, "WebhookTask", 8192, (void*)payloadStr, 1, NULL);
        }

        saveTotalToNVS();
        publishMqttTelemetry();
    }

    if (autoCutoff) {
        Serial.println("[DISPENSE] TARGET TERCAPAI! Auto Cut-off Solenoid Valve aktif.");
        beepBuzzer(150, 3);
    } else if (strcmp(stopReason, "dry_run") == 0) {
        Serial.println("[DISPENSE] DRY RUN ALARM! Pompa kosong / tidak ada aliran.");
        beepBuzzer(500, 2);
    } else if (wasDispensing) {
        Serial.println("[DISPENSE] Pengisian selesai/dihentikan.");
        beepBuzzer(250, 1);
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
    doc["valve_fast_open"] = valveFastOpen;
    doc["valve_slow_open"] = valveSlowOpen;
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
    doc["active_fluid_id"] = activeFluidId;

    // Two-Stage Valve
    doc["two_stage_enabled"] = twoStageEnabled;
    doc["slow_stage_liters"] = slowStageLiters;

    // Smart Tank Inventory
    doc["tank_capacity"] = tankCapacityLiters;
    doc["tank_remaining"] = tankRemainingLiters;
    doc["tank_low_alert"] = tankLowAlertLiters;
    doc["tank_percentage"] = (tankCapacityLiters > 0.0f) ? ((tankRemainingLiters / tankCapacityLiters) * 100.0f) : 0.0f;
    doc["tank_low_alarm"] = tankLowAlarm;

    // MQTT Telemetry Status
    doc["mqtt_enabled"] = mqttEnabled;
    doc["mqtt_connected"] = mqttClient.connected();
    doc["mqtt_broker"] = mqttBroker;
    doc["mqtt_port"] = mqttPort;
    doc["mqtt_topic"] = mqttTopic;

    // Rekap Shift Operator
    doc["shift_liters"] = shiftLiters;
    doc["shift_cost"] = shiftCost;
    doc["shift_tx_count"] = shiftTxCount;

    // Rekap Harian
    doc["today_liters"] = todayLiters;
    doc["today_cost"] = todayCost;
    doc["today_tx_count"] = todayTxCount;
    doc["today_date"] = todayDateStr;

    // Multi-Point Calibration Metrics
    doc["mp_enabled"] = multiPointCalibEnabled;
    doc["k_low"] = kLow;
    doc["k_mid"] = kMid;
    doc["k_high"] = kHigh;
    doc["effective_k"] = getEffectiveKFactor(currentFlowRateLpm);

    // Struk Instan Terakhir
    doc["last_liters"] = lastBatchLiters;
    doc["last_cost"] = lastBatchCost;
    doc["last_time"] = lastBatchTimeStr;
    doc["last_id"] = lastBatchId;
    doc["store_name"] = storeName;
    doc["store_address"] = storeAddress;
    doc["store_footer"] = storeFooter;

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void handleResetShift() {
    shiftLiters = 0.0f;
    shiftCost = 0.0f;
    shiftTxCount = 0;
    preferences.begin("flowmeter", false);
    preferences.putFloat("s_liters", 0.0f);
    preferences.putFloat("s_cost", 0.0f);
    preferences.putUInt("s_count", 0);
    preferences.end();
    beepBuzzer(120, 2);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Shift operator berhasil di-reset ke 0\"}");
}

void handleFluidPreset() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String presetId = doc["preset_id"] | "";

    bool found = false;
    for (size_t i = 0; i < BUILTIN_FLUIDS_COUNT; i++) {
        if (presetId == BUILTIN_FLUIDS[i].id) {
            fluidName = BUILTIN_FLUIDS[i].name;
            pricePerLiter = BUILTIN_FLUIDS[i].price;
            kFactor = BUILTIN_FLUIDS[i].kFactor;
            kLow = BUILTIN_FLUIDS[i].kFactor * 0.99f;
            kMid = BUILTIN_FLUIDS[i].kFactor;
            kHigh = BUILTIN_FLUIDS[i].kFactor * 1.01f;
            overshootLiters = BUILTIN_FLUIDS[i].overshoot;
            activeFluidId = BUILTIN_FLUIDS[i].id;
            found = true;
            break;
        }
    }

    if (!found) {
        if (doc["name"].is<String>()) fluidName = doc["name"].as<String>();
        if (doc["price"].is<float>()) pricePerLiter = doc["price"].as<float>();
        if (doc["k_factor"].is<float>()) kFactor = doc["k_factor"].as<float>();
        if (doc["overshoot"].is<float>()) overshootLiters = doc["overshoot"].as<float>();
        activeFluidId = presetId;
    }

    saveConfigToNVS();
    beepBuzzer(150, 1);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Profil cairan berhasil diaktifkan\",\"fluid\":\"" + fluidName + "\"}");
}

void handleCalibrateMultiPoint() {
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

    if (doc["k_low"].is<float>()) kLow = doc["k_low"].as<float>();
    if (doc["k_mid"].is<float>()) kMid = doc["k_mid"].as<float>();
    if (doc["k_high"].is<float>()) kHigh = doc["k_high"].as<float>();
    if (doc["enabled"].is<bool>()) multiPointCalibEnabled = doc["enabled"].as<bool>();

    preferences.begin("flowmeter", false);
    preferences.putBool("mp_en", multiPointCalibEnabled);
    preferences.putFloat("k_low", kLow);
    preferences.putFloat("k_mid", kMid);
    preferences.putFloat("k_high", kHigh);
    preferences.end();

    beepBuzzer(150, 2);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Kalibrasi multi-titik disimpan!\"}");
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
    if (valveOpen || isDispensing) {
        server.send(400, "application/json", "{\"error\":\"Tidak dapat mereset saat cairan sedang mengalir! Hentikan pengisian terlebih dahulu.\"}");
        return;
    }
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
        if (newK > 0.0f) {
            kFactor = newK;
            kMid = newK;
            kLow = newK * 0.99f;
            kHigh = newK * 1.01f;
        }
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
    kMid = newK;
    kLow = newK * 0.99f;
    kHigh = newK * 1.01f;
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

// Smart Tank Endpoints
void handleTankGet() {
    JsonDocument doc;
    doc["capacity"] = tankCapacityLiters;
    doc["remaining"] = tankRemainingLiters;
    doc["low_alert"] = tankLowAlertLiters;
    doc["percentage"] = (tankCapacityLiters > 0.0f) ? ((tankRemainingLiters / tankCapacityLiters) * 100.0f) : 0.0f;
    doc["is_low"] = tankLowAlarm;
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleTankRefill() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload kosong\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));

    if (doc["added_liters"].is<float>()) {
        float added = doc["added_liters"].as<float>();
        tankRemainingLiters += added;
        if (tankRemainingLiters > tankCapacityLiters) tankRemainingLiters = tankCapacityLiters;
    } else if (doc["set_remaining"].is<float>()) {
        tankRemainingLiters = doc["set_remaining"].as<float>();
        if (tankRemainingLiters > tankCapacityLiters) tankRemainingLiters = tankCapacityLiters;
    }

    tankLowAlarm = (tankRemainingLiters <= tankLowAlertLiters);
    nvsDirty = true;
    saveTotalToNVS();
    beepBuzzer(150, 2);

    JsonDocument resp;
    resp["status"] = "ok";
    resp["remaining"] = tankRemainingLiters;
    resp["capacity"] = tankCapacityLiters;
    resp["message"] = "Stok tangki berhasil diperbarui";
    String out;
    serializeJson(resp, out);
    server.send(200, "application/json", out);
}

void handleTankSettings() {
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

    if (doc["capacity"].is<float>()) tankCapacityLiters = doc["capacity"].as<float>();
    if (doc["low_alert"].is<float>()) tankLowAlertLiters = doc["low_alert"].as<float>();
    if (doc["remaining"].is<float>()) tankRemainingLiters = doc["remaining"].as<float>();

    tankLowAlarm = (tankRemainingLiters <= tankLowAlertLiters);
    saveConfigToNVS();
    beepBuzzer(150, 1);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Pengaturan kapasitas tangki disimpan\"}");
}

// Two-Stage Valve Settings
void handleValveSettings() {
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

    if (doc["two_stage_enabled"].is<bool>()) twoStageEnabled = doc["two_stage_enabled"].as<bool>();
    if (doc["slow_stage_liters"].is<float>()) slowStageLiters = doc["slow_stage_liters"].as<float>();

    saveConfigToNVS();
    beepBuzzer(150, 1);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Pengaturan katup dua tahap disimpan\"}");
}

// MQTT Telemetry Settings
void handleMqttSettings() {
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

    if (doc["enabled"].is<bool>()) mqttEnabled = doc["enabled"].as<bool>();
    if (doc["broker"].is<String>()) mqttBroker = doc["broker"].as<String>();
    if (doc["port"].is<uint16_t>()) mqttPort = doc["port"].as<uint16_t>();
    if (doc["user"].is<String>()) mqttUser = doc["user"].as<String>();
    if (doc["pass"].is<String>()) mqttPass = doc["pass"].as<String>();
    if (doc["topic"].is<String>()) mqttTopic = doc["topic"].as<String>();

    saveConfigToNVS();
    setupMqtt();
    mqttHaDiscoverySent = false;
    lastMqttReconnect = 0;

    beepBuzzer(150, 1);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Pengaturan MQTT disimpan\"}");
}

// Thermal Printer Receipt Handlers
void handlePrintReceipt() {
    uint32_t id = lastBatchId;
    float vol = lastBatchLiters;
    float cost = lastBatchCost;
    float price = pricePerLiter;
    String timeStr = lastBatchTimeStr;
    String fluid = fluidName;
    String mode = lastBatchMode;

    if (server.hasArg("id")) {
        uint32_t queryId = server.arg("id").toInt();
        for (int i = 0; i < historyManager.getCount(); i++) {
            const TransactionRecord* rec = historyManager.getRecord(i);
            if (rec && rec->id == queryId) {
                id = rec->id;
                vol = rec->volume;
                cost = rec->cost;
                timeStr = rec->formattedTime;
                fluid = rec->fluidName;
                mode = rec->mode;
                price = (vol > 0.001f) ? (cost / vol) : pricePerLiter;
                break;
            }
        }
    }

    String format = server.hasArg("format") ? server.arg("format") : "escpos";
    if (format == "text") {
        String txt = generateTextReceipt(id, timeStr, fluid, vol, price, cost, mode);
        server.send(200, "text/plain", txt);
    } else {
        String esc = generateEscPosReceipt(id, timeStr, fluid, vol, price, cost, mode);
        server.sendHeader("Content-Disposition", "inline; filename=\"receipt_" + String(id) + ".bin\"");
        server.send(200, "application/octet-stream", esc);
    }
}

void handlePrintSerial() {
    uint32_t id = lastBatchId;
    float vol = lastBatchLiters;
    float cost = lastBatchCost;
    float price = pricePerLiter;
    String timeStr = lastBatchTimeStr;
    String fluid = fluidName;
    String mode = lastBatchMode;

    if (server.hasArg("id")) {
        uint32_t queryId = server.arg("id").toInt();
        for (int i = 0; i < historyManager.getCount(); i++) {
            const TransactionRecord* rec = historyManager.getRecord(i);
            if (rec && rec->id == queryId) {
                id = rec->id;
                vol = rec->volume;
                cost = rec->cost;
                timeStr = rec->formattedTime;
                fluid = rec->fluidName;
                mode = rec->mode;
                price = (vol > 0.001f) ? (cost / vol) : pricePerLiter;
                break;
            }
        }
    }

    String esc = generateEscPosReceipt(id, timeStr, fluid, vol, price, cost, mode);
    Serial.print(esc); // Kirim stream binary ESC/POS langsung ke Hardware Serial

    beepBuzzer(100, 2);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Struk berhasil dikirim ke printer Serial\"}");
}

void handlePrintSettings() {
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

    if (doc["store_name"].is<String>()) storeName = doc["store_name"].as<String>();
    if (doc["store_address"].is<String>()) storeAddress = doc["store_address"].as<String>();
    if (doc["store_footer"].is<String>()) storeFooter = doc["store_footer"].as<String>();

    saveConfigToNVS();
    beepBuzzer(150, 1);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Pengaturan header & footer struk disimpan\"}");
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
        todayDateStr = getCurrentDateString();
        Serial.printf("[TIME] Jam RTC disinkronkan dari browser: %s (%s)\n", getFormattedCurrentTime().c_str(), todayDateStr.c_str());
        server.send(200, "application/json", "{\"status\":\"ok\",\"time\": \"" + getFormattedCurrentTime() + "\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Timestamp epoch tidak valid\"}");
    }
}

// Backup Konfigurasi (JSON Export)
void handleConfigBackup() {
    JsonDocument doc;
    doc["k_factor"] = kFactor;
    doc["price_per_liter"] = pricePerLiter;
    doc["fluid_name"] = fluidName;
    doc["unit_name"] = unitName;
    doc["currency"] = currencySymbol;
    doc["active_fluid_id"] = activeFluidId;
    doc["overshoot_liters"] = overshootLiters;
    doc["webhook_url"] = webhookUrl;
    doc["sta_ssid"] = staSsid;
    doc["mp_enabled"] = multiPointCalibEnabled;
    doc["k_low"] = kLow;
    doc["k_mid"] = kMid;
    doc["k_high"] = kHigh;
    doc["two_stage_enabled"] = twoStageEnabled;
    doc["slow_stage_liters"] = slowStageLiters;
    doc["tank_capacity"] = tankCapacityLiters;
    doc["tank_remaining"] = tankRemainingLiters;
    doc["tank_low_alert"] = tankLowAlertLiters;
    doc["store_name"] = storeName;
    doc["store_address"] = storeAddress;
    doc["store_footer"] = storeFooter;
    doc["mqtt_enabled"] = mqttEnabled;
    doc["mqtt_broker"] = mqttBroker;
    doc["mqtt_port"] = mqttPort;
    doc["mqtt_topic"] = mqttTopic;
    doc["backup_time"] = getFormattedCurrentTime();

    String out;
    serializeJsonPretty(doc, out);
    server.sendHeader("Content-Disposition", "attachment; filename=\"flowmeter_backup_config.json\"");
    server.send(200, "application/json", out);
}

// Restore Konfigurasi (JSON Import)
void handleConfigRestore() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Payload backup kosong!\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    String pin = doc["pin"] | "";
    if (pin != adminPin) {
        server.send(403, "application/json", "{\"error\":\"PIN Admin salah!\"}");
        return;
    }

    if (doc["k_factor"].is<float>()) kFactor = doc["k_factor"].as<float>();
    if (doc["price_per_liter"].is<float>()) pricePerLiter = doc["price_per_liter"].as<float>();
    if (doc["fluid_name"].is<String>()) fluidName = doc["fluid_name"].as<String>();
    if (doc["unit_name"].is<String>()) unitName = doc["unit_name"].as<String>();
    if (doc["currency"].is<String>()) currencySymbol = doc["currency"].as<String>();
    if (doc["active_fluid_id"].is<String>()) activeFluidId = doc["active_fluid_id"].as<String>();
    if (doc["overshoot_liters"].is<float>()) overshootLiters = doc["overshoot_liters"].as<float>();
    if (doc["webhook_url"].is<String>()) webhookUrl = doc["webhook_url"].as<String>();
    if (doc["sta_ssid"].is<String>()) staSsid = doc["sta_ssid"].as<String>();
    if (doc["mp_enabled"].is<bool>()) multiPointCalibEnabled = doc["mp_enabled"].as<bool>();
    if (doc["k_low"].is<float>()) kLow = doc["k_low"].as<float>();
    if (doc["k_mid"].is<float>()) kMid = doc["k_mid"].as<float>();
    if (doc["k_high"].is<float>()) kHigh = doc["k_high"].as<float>();
    if (doc["two_stage_enabled"].is<bool>()) twoStageEnabled = doc["two_stage_enabled"].as<bool>();
    if (doc["slow_stage_liters"].is<float>()) slowStageLiters = doc["slow_stage_liters"].as<float>();
    if (doc["tank_capacity"].is<float>()) tankCapacityLiters = doc["tank_capacity"].as<float>();
    if (doc["tank_remaining"].is<float>()) tankRemainingLiters = doc["tank_remaining"].as<float>();
    if (doc["tank_low_alert"].is<float>()) tankLowAlertLiters = doc["tank_low_alert"].as<float>();
    if (doc["store_name"].is<String>()) storeName = doc["store_name"].as<String>();
    if (doc["store_address"].is<String>()) storeAddress = doc["store_address"].as<String>();
    if (doc["store_footer"].is<String>()) storeFooter = doc["store_footer"].as<String>();
    if (doc["mqtt_enabled"].is<bool>()) mqttEnabled = doc["mqtt_enabled"].as<bool>();
    if (doc["mqtt_broker"].is<String>()) mqttBroker = doc["mqtt_broker"].as<String>();
    if (doc["mqtt_port"].is<uint16_t>()) mqttPort = doc["mqtt_port"].as<uint16_t>();
    if (doc["mqtt_topic"].is<String>()) mqttTopic = doc["mqtt_topic"].as<String>();

    saveConfigToNVS();
    setupMqtt();
    beepBuzzer(150, 2);
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Konfigurasi berhasil dipulihkan (restored)!\"}");
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
    if (doc["webhook"].is<String>()) {
        webhookUrl = doc["webhook"].as<String>();
    }
    saveConfigToNVS();

    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"WiFi tersimpan, mencoba menghubungkan...\"}");

    if (staSsid.length() > 0) {
        WiFi.begin(staSsid.c_str(), staPassword.c_str());
    }
}

void handleNotFound() {
    if (server.uri().startsWith("/api/")) {
        server.send(404, "application/json", "{\"error\":\"Endpoint API tidak ditemukan\"}");
        return;
    }
    server.sendHeader("Location", String("http://") + AP_IP_ADDRESS, true);
    server.send(302, "text/plain", "");
}

// ==========================================
// ARDUINO SETUP
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n==============================================");
    Serial.println("  UNIVERSAL FLOW METER ESP32-S3 | SMART NVR");
    Serial.println("==============================================");

    // Setup GPIO
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(LED_FLOW_PIN, OUTPUT);
    setFlowLed(false);

    pinMode(RELAY_VALVE_PIN, OUTPUT);
    pinMode(RELAY_VALVE_SLOW_PIN, OUTPUT);
    setValves(false, false);

    buzzer.begin();

    // Inisialisasi Layar LCD 20x4 I2C (Deteksi Otomatis 0x27 / 0x3F)
    flowLcd.begin();

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
        Serial.printf("[WIFI] Mencoba konek ke WiFi: %s\n", staSsid.c_str());
        WiFi.begin(staSsid.c_str(), staPassword.c_str());
    }

    // 3. Setup SNTP Time Synchronization (GMT+7 WIB)
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    // 4. Setup MQTT Client
    setupMqtt();

    // DNS Server untuk Captive Portal
    dnsServer.start(53, "*", local_ip);

    // Web Server Endpoints
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/data", HTTP_GET, handleApiData);
    server.on("/api/dispense/start", HTTP_POST, handleDispenseStart);
    server.on("/api/dispense/stop", HTTP_POST, handleDispenseStop);
    server.on("/api/reset_batch", HTTP_POST, handleResetBatch);
    server.on("/api/reset_total", HTTP_POST, handleResetTotal);
    server.on("/api/shift/reset", HTTP_POST, handleResetShift);
    server.on("/api/fluid/preset", HTTP_POST, handleFluidPreset);
    server.on("/api/calibrate/multipoint", HTTP_POST, handleCalibrateMultiPoint);
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
    server.on("/api/config/backup", HTTP_GET, handleConfigBackup);
    server.on("/api/config/restore", HTTP_POST, handleConfigRestore);

    // Smart Tank Inventory Endpoints
    server.on("/api/tank", HTTP_GET, handleTankGet);
    server.on("/api/tank/refill", HTTP_POST, handleTankRefill);
    server.on("/api/tank/settings", HTTP_POST, handleTankSettings);

    // Two-Stage Valve Settings Endpoint
    server.on("/api/valve/settings", HTTP_POST, handleValveSettings);

    // MQTT Settings Endpoint
    server.on("/api/mqtt/settings", HTTP_POST, handleMqttSettings);

    // ESC/POS Thermal Receipt Printer Endpoints
    server.on("/api/print/receipt", HTTP_GET, handlePrintReceipt);
    server.on("/api/print/serial", HTTP_POST, handlePrintSerial);
    server.on("/api/print/settings", HTTP_POST, handlePrintSettings);

    // Web OTA Firmware Update via Browser
    server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "application/json", Update.hasError() ? "{\"status\":\"error\",\"message\":\"Update gagal! Cek format firmware.bin\"}" : "{\"status\":\"ok\",\"message\":\"Update berhasil! ESP32 sedang restart...\"}");
        delay(500);
        ESP.restart();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("[OTA] Update dimulai: %s\n", upload.filename.c_str());
            if (valveOpen) {
                setValve(false);
            }
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                Serial.printf("[OTA] Selesai flash firmware! Ukuran: %u bytes\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[HTTP] Web Server aktif dan siap melayani permintaan.");

    beepBuzzer(80, 2);
    lastCalcTime = millis();
}

// ==========================================
// ARDUINO MAIN LOOP
// ==========================================
void loop() {
    dnsServer.processNextRequest();
    server.handleClient();

    unsigned long currentMillis = millis();

    // Update non-blocking buzzer state
    buzzer.update(currentMillis);

    // Check STA connection status
    staConnected = (WiFi.status() == WL_CONNECTED);

    // Handle MQTT connection & telemetry
    handleMqtt(currentMillis);

    // Perhitungan Debit & Volume setiap 300 ms (fast response)
    if (currentMillis - lastCalcTime >= 300) {
        unsigned long deltaMs = currentMillis - lastCalcTime;
        lastCalcTime = currentMillis;

        noInterrupts();
        uint32_t pulses = isrPulseCount;
        isrPulseCount = 0;
        interrupts();

        if (pulses > 0) {
            lastPulseSeenMillis = currentMillis;
            isDispensing = true;
            setFlowLed(true);
        } else {
            if (currentMillis - lastPulseSeenMillis > 1200) {
                isDispensing = false;
                setFlowLed(false);
            }
        }

        // Hitung debit aliran dinamis (Flow Rate LPM)
        float effK = getEffectiveKFactor(currentFlowRateLpm);
        if (isDispensing && effK > 0.0f) {
            float volumeThisPeriod = (float)pulses / effK;
            currentFlowRateLpm = (volumeThisPeriod * 60000.0f) / (float)deltaMs;
            totalLiters += (double)volumeThisPeriod;
            
            // Akumulasi batch inkremental (bebas distorsi multi-point kalibrasi dinamis)
            batchLiters += volumeThisPeriod;
            batchCost = batchLiters * pricePerLiter;

            // Pengurangan stok tangki secara real-time
            if (tankRemainingLiters > volumeThisPeriod) {
                tankRemainingLiters -= volumeThisPeriod;
            } else {
                tankRemainingLiters = 0.0f;
            }
            tankLowAlarm = (tankRemainingLiters <= tankLowAlertLiters);
            nvsDirty = true;
        } else {
            currentFlowRateLpm = 0.0f;
        }

        // ==========================================
        // AUTO CUT-OFF EVALUATION (TWO-STAGE INCHING & OVERSHOOT)
        // ==========================================
        if (valveOpen) {
            float effectiveCutoffVolume = targetDispenseVolume > overshootLiters ? (targetDispenseVolume - overshootLiters) : targetDispenseVolume;
            float effectiveCutoffCost = targetDispenseAmount > (overshootLiters * pricePerLiter) ? (targetDispenseAmount - (overshootLiters * pricePerLiter)) : targetDispenseAmount;

            // Transisi Katup Tahap 2 (Inching / Slow-Flow):
            // Matikan katup utama (fast) dan biarkan katup kecil (slow) menyala saat mendekati target
            if (twoStageEnabled && valveFastOpen) {
                if (activeDispenseMode == MODE_PRESET_VOLUME && targetDispenseVolume > (slowStageLiters * 1.5f)) {
                    float remainingVol = targetDispenseVolume - batchLiters;
                    if (remainingVol <= slowStageLiters) {
                        valveFastOpen = false;
                        digitalWrite(RELAY_VALVE_PIN, RELAY_INACTIVE_LEVEL);
                        Serial.printf("[VALVE] Inching Stage Aktif! Sisa=%.3f L. Katup utama OFF, katup pelan ON.\n", remainingVol);
                    }
                } else if (activeDispenseMode == MODE_PRESET_AMOUNT && targetDispenseAmount > (slowStageLiters * pricePerLiter * 1.5f)) {
                    float remainingCost = targetDispenseAmount - batchCost;
                    if (remainingCost <= (slowStageLiters * pricePerLiter)) {
                        valveFastOpen = false;
                        digitalWrite(RELAY_VALVE_PIN, RELAY_INACTIVE_LEVEL);
                        Serial.printf("[VALVE] Inching Stage Aktif! Sisa=Rp %.0f. Katup pelan ON.\n", remainingCost);
                    }
                }
            }

            // Evaluasi Auto Cut-off Akhir
            if (activeDispenseMode == MODE_PRESET_VOLUME && targetDispenseVolume > 0.0f) {
                if (batchLiters >= effectiveCutoffVolume) {
                    stopDispense(true, "auto_cutoff");
                }
            } else if (activeDispenseMode == MODE_PRESET_AMOUNT && targetDispenseAmount > 0.0f) {
                if (batchCost >= effectiveCutoffCost) {
                    stopDispense(true, "auto_cutoff");
                }
            }

            // SAFETY: Dry-Run Protection (Jika valve buka tapi tidak ada aliran)
            unsigned long dryTimeout = (activeDispenseMode == MODE_MANUAL) ? 20000 : 8000;
            if (currentMillis - dispenseStartTime >= dryTimeout) {
                if (currentFlowRateLpm < 0.15f && batchLiters < 0.05f) {
                    dryRunAlarm = true;
                    stopDispense(false, "dry_run");
                }
            }
        } else {
            // Dalam mode standalone tanpa valve / manual flowmeter, aliran bebas diizinkan
            // Hanya picu alarm bocor jika mode preset aktif dan katup ditutup sengaja
            idlePulseAccumulator = 0;
        }
    }

    // Safety Alarm Buzzer Beep (Peringatan Non-blocking)
    if (dryRunAlarm || leakageAlert) {
        if (currentMillis - lastAlarmBeepMillis >= 1500) {
            lastAlarmBeepMillis = currentMillis;
            beepBuzzer(40, 1);
        }
    }

    // Auto-save total akumulasi ke NVS per 10 detik dengan dirty flag (Wear-Leveling Protection)
    if (currentMillis - lastNvsSaveMillis >= NVS_AUTO_SAVE_INTERVAL) {
        lastNvsSaveMillis = currentMillis;
        if (!isDispensing && !valveOpen && nvsDirty) {
            saveTotalToNVS();
        }
    }

    // Update Layar LCD 20x4 I2C Secara Real-Time (Tanpa Flicker)
    String activeDisplayIp = (staConnected && WiFi.localIP().toString() != "0.0.0.0") ? WiFi.localIP().toString() : AP_IP_ADDRESS;
    flowLcd.update(batchLiters, batchCost, currentFlowRateLpm, valveOpen, isDispensing, fluidName, pricePerLiter, activeDisplayIp, dryRunAlarm, leakageAlert, tankLowAlarm, tankRemainingLiters);
}
