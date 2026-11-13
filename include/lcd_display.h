#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

class FlowMeterLcd {
private:
    LiquidCrystal_I2C* lcd = nullptr;
    bool available = false;
    uint8_t i2cAddr = 0;
    unsigned long lastUpdateMillis = 0;
    char lineBuffer[21]; // 20 chars + null terminator

    uint8_t scanI2C() {
        // Cek dulu alamat umum PCF8574: 0x27 dan 0x3F
        uint8_t commonAddrs[] = {0x27, 0x3F, 0x26, 0x38, 0x20};
        for (uint8_t addr : commonAddrs) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                return addr;
            }
        }
        // Scan menyeluruh jika belum ketemu
        for (uint8_t addr = 0x20; addr <= 0x3F; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                return addr;
            }
        }
        return 0; // Tidak ditemukan
    }

public:
    void begin() {
        Serial.printf("[LCD] Menginisialisasi I2C (SDA=GPIO %d, SCL=GPIO %d)...\n", I2C_SDA_PIN, I2C_SCL_PIN);
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        Wire.setClock(100000); // 100 kHz standar I2C yang stabil

        i2cAddr = scanI2C();
        if (i2cAddr == 0) {
            Serial.println("[LCD] Peringatan: Layar LCD I2C 20x4 tidak terdeteksi pada bus I2C.");
            Serial.println("      Periksa kabel VCC (5V), GND, SDA, SCL dan trimpot kontras.");
            available = false;
            return;
        }

        Serial.printf("[LCD] Layar LCD I2C terdeteksi pada alamat 0x%02X!\n", i2cAddr);
        lcd = new LiquidCrystal_I2C(i2cAddr, LCD_COLS, LCD_ROWS);
        lcd->init();
        lcd->backlight();
        lcd->clear();

        // Tampilan Sambutan / Booting
        printLine(0, " UNIVERSAL FLOWMETER ");
        printLine(1, " DIGITAL DISPENSER   ");
        #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(ARDUINO_ESP32C3_DEV)
        printLine(2, " ESP32-C3 SUPERMINI  ");
        #else
        printLine(2, " ESP32-S3 DEVKITC-1  ");
        #endif
        printLine(3, " IP: 192.168.4.1     ");

        available = true;
    }

    bool isAvailable() const {
        return available;
    }

    void printLine(uint8_t row, const char* str) {
        if (!available || !lcd || row >= LCD_ROWS) return;
        snprintf(lineBuffer, sizeof(lineBuffer), "%-20.20s", str);
        lcd->setCursor(0, row);
        lcd->print(lineBuffer);
    }

    void update(float batchLiters, float batchCost, float flowRateLpm, bool valveOpen, bool isDispensing,
                const String& fluid, float pricePerLiter, const String& ipStr, bool dryRun, bool leakage) {
        if (!available || !lcd) return;

        unsigned long now = millis();
        if (now - lastUpdateMillis < 250) return; // Refresh rate ~4 FPS (tanpa flicker)
        lastUpdateMillis = now;

        // Baris 0: Status / Nama Cairan & Harga
        char row0[21];
        if (dryRun) {
            snprintf(row0, sizeof(row0), "** ALARM DRY RUN! **");
        } else if (leakage) {
            snprintf(row0, sizeof(row0), "** ALARM BOCOR!   **");
        } else {
            // Contoh: "SOLAR  Rp6.800/L    "
            String shortName = fluid.substring(0, 10);
            snprintf(row0, sizeof(row0), "%-9s @Rp%-6.0f", shortName.c_str(), pricePerLiter);
        }
        printLine(0, row0);

        // Baris 1: Volume Liter Pengisian Saat Ini
        // Contoh: "VOL :     12.345 L  "
        char row1[21];
        snprintf(row1, sizeof(row1), "VOL : %10.3f L ", batchLiters);
        printLine(1, row1);

        // Baris 2: Total Rupiah / Biaya
        // Contoh: "RP  : Rp      83.946"
        char row2[21];
        snprintf(row2, sizeof(row2), "RP  : Rp %9.0f  ", batchCost);
        printLine(2, row2);

        // Baris 3: Laju Debit Aliran (LPM) & Status Valve
        // Contoh: "FLOW: 12.4 LPM [ON] " atau "IP: 192.168.4.1 [OFF]"
        char row3[21];
        const char* vStatus = valveOpen ? "[ON] " : (isDispensing ? "[FLO]" : "[OFF]");
        if (valveOpen || isDispensing || flowRateLpm > 0.05f) {
            snprintf(row3, sizeof(row3), "RATE:%5.1f LPM %s", flowRateLpm, vStatus);
        } else {
            snprintf(row3, sizeof(row3), "IP:%-12s%s", ipStr.substring(0, 12).c_str(), vStatus);
        }
        printLine(3, row3);
    }
};

extern FlowMeterLcd flowLcd;

#endif // LCD_DISPLAY_H

