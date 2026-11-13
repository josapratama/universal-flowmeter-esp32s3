#ifndef HISTORY_H
#define HISTORY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include "config.h"

struct TransactionRecord {
    uint32_t id;
    uint32_t epochTime;          // Unix epoch timestamp (NTP or browser synced)
    uint32_t timestampSeconds;   // Uptime fallback
    char formattedTime[24];      // "YYYY-MM-DD HH:MM:SS"
    char fluidName[24];
    float volume;
    char unitName[12];
    float cost;
    char currency[8];
    char mode[16];               // "manual", "preset_volume", "preset_amount"
};

class TransactionHistoryManager {
private:
    TransactionRecord records[MAX_HISTORY_RECORDS];
    uint16_t count = 0;
    uint32_t nextId = 1;
    Preferences pref;

public:
    void begin() {
        pref.begin("flow_hist", false);
        count = pref.getUShort("count", 0);
        nextId = pref.getUInt("next_id", 1);
        if (count > MAX_HISTORY_RECORDS) count = MAX_HISTORY_RECORDS;

        // Load records from NVS blob
        size_t expectedSize = count * sizeof(TransactionRecord);
        if (count > 0 && expectedSize > 0) {
            pref.getBytes("records", records, expectedSize);
        }
        pref.end();
        Serial.printf("[HISTORY] Dimuat: %d transaksi terakhir tersimpan di Flash.\n", count);
    }

    void addTransaction(const char* fluid, float vol, const char* unit, float cost, const char* curr, const char* mode) {
        if (vol <= 0.001f) return; // Abaikan jika 0

        TransactionRecord rec;
        rec.id = nextId++;
        rec.timestampSeconds = (uint32_t)(millis() / 1000);

        // Get actual calendar time if RTC / NTP synced
        time_t nowEpoch = time(nullptr);
        rec.epochTime = (uint32_t)nowEpoch;

        if (nowEpoch > 1700000000) { // Valid timestamp after Nov 2023
            struct tm timeinfo;
            localtime_r(&nowEpoch, &timeinfo);
            strftime(rec.formattedTime, sizeof(rec.formattedTime), "%Y-%m-%d %H:%M:%S", &timeinfo);
        } else {
            // Fallback uptime format
            uint32_t hrs = rec.timestampSeconds / 3600;
            uint32_t mins = (rec.timestampSeconds % 3600) / 60;
            uint32_t secs = rec.timestampSeconds % 60;
            snprintf(rec.formattedTime, sizeof(rec.formattedTime), "Uptime %02u:%02u:%02u", 
                     (unsigned int)hrs, (unsigned int)mins, (unsigned int)secs);
        }

        strncpy(rec.fluidName, fluid, sizeof(rec.fluidName) - 1);
        rec.fluidName[sizeof(rec.fluidName) - 1] = '\0';
        rec.volume = vol;
        strncpy(rec.unitName, unit, sizeof(rec.unitName) - 1);
        rec.unitName[sizeof(rec.unitName) - 1] = '\0';
        rec.cost = cost;
        strncpy(rec.currency, curr, sizeof(rec.currency) - 1);
        rec.currency[sizeof(rec.currency) - 1] = '\0';
        strncpy(rec.mode, mode, sizeof(rec.mode) - 1);
        rec.mode[sizeof(rec.mode) - 1] = '\0';

        // Shift oldest if full
        if (count >= MAX_HISTORY_RECORDS) {
            for (int i = 0; i < MAX_HISTORY_RECORDS - 1; i++) {
                records[i] = records[i + 1];
            }
            records[MAX_HISTORY_RECORDS - 1] = rec;
        } else {
            records[count++] = rec;
        }

        save();
        Serial.printf("[HISTORY] Transaksi #%u dicatat: %.3f %s (%s %.0f) - %s\n", 
                      (unsigned int)rec.id, vol, unit, curr, cost, rec.formattedTime);
    }

    void save() {
        pref.begin("flow_hist", false);
        pref.putUShort("count", count);
        pref.putUInt("next_id", nextId);
        size_t size = count * sizeof(TransactionRecord);
        if (size > 0) {
            pref.putBytes("records", records, size);
        }
        pref.end();
    }

    void clear() {
        count = 0;
        nextId = 1;
        memset(records, 0, sizeof(records));
        pref.begin("flow_hist", false);
        pref.clear();
        pref.end();
        Serial.println("[HISTORY] Riwayat transaksi dikosongkan.");
    }

    uint16_t getCount() const { return count; }

    const TransactionRecord* getRecord(int index) const {
        if (index < 0 || index >= count) return nullptr;
        return &records[index];
    }

    String toJson() const {
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();

        // Return latest first
        for (int i = count - 1; i >= 0; i--) {
            JsonObject obj = arr.add<JsonObject>();
            obj["id"] = records[i].id;
            obj["time_str"] = records[i].formattedTime;
            obj["epoch"] = records[i].epochTime;
            obj["uptime_sec"] = records[i].timestampSeconds;
            obj["fluid"] = records[i].fluidName;
            obj["volume"] = records[i].volume;
            obj["unit"] = records[i].unitName;
            obj["cost"] = records[i].cost;
            obj["currency"] = records[i].currency;
            obj["mode"] = records[i].mode;
        }

        String output;
        serializeJson(doc, output);
        return output;
    }

    String toCsv() const {
        String csv = "ID,Waktu_Transaksi,Epoch_Unix,Uptime_Detik,Cairan,Volume,Satuan,Total_Harga,Mata_Uang,Mode_Pengisian\r\n";
        for (int i = count - 1; i >= 0; i--) {
            csv += String((unsigned int)records[i].id) + ",";
            csv += "\"" + String(records[i].formattedTime) + "\",";
            csv += String(records[i].epochTime) + ",";
            csv += String(records[i].timestampSeconds) + ",";
            csv += "\"" + String(records[i].fluidName) + "\",";
            csv += String(records[i].volume, 3) + ",";
            csv += String(records[i].unitName) + ",";
            csv += String((long)records[i].cost) + ",";
            csv += String(records[i].currency) + ",";
            csv += String(records[i].mode) + "\r\n";
        }
        return csv;
    }
};

extern TransactionHistoryManager historyManager;

#endif // HISTORY_H
