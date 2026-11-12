#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Digital Flow Meter & Dispenser | ESP32-S3</title>
  <style>
    :root {
      --bg: #090d16;
      --card-bg: #131b2e;
      --card-border: #1e293b;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --primary: #0ea5e9;
      --primary-hover: #0284c7;
      --success: #10b981;
      --danger: #ef4444;
      --amber: #f59e0b;
      --disp-bg: #030712;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 14px; min-height: 100vh; display: flex; flex-direction: column; align-items: center; }
    .container { width: 100%; max-width: 860px; display: flex; flex-direction: column; gap: 14px; }
    
    /* Header */
    header { display: flex; justify-content: space-between; align-items: center; padding: 14px 18px; background: var(--card-bg); border-radius: 16px; border: 1px solid var(--card-border); flex-wrap: wrap; gap: 10px; }
    .brand { display: flex; align-items: center; gap: 12px; }
    .brand-icon { width: 44px; height: 44px; background: linear-gradient(135deg, #0ea5e9, #6366f1); border-radius: 12px; display: flex; align-items: center; justify-content: center; font-size: 22px; box-shadow: 0 4px 14px rgba(14, 165, 233, 0.4); }
    .brand-text h1 { font-size: 1.15rem; font-weight: 700; }
    .fluid-badge { display: inline-block; font-size: 0.75rem; font-weight: 700; padding: 2px 8px; border-radius: 6px; background: rgba(245, 158, 11, 0.15); color: #f59e0b; border: 1px solid rgba(245, 158, 11, 0.3); margin-top: 2px; }
    .header-badges { display: flex; align-items: center; gap: 8px; flex-wrap: wrap; }
    .badge { padding: 5px 12px; border-radius: 9999px; font-size: 0.72rem; font-weight: 700; display: flex; align-items: center; gap: 6px; }
    .badge-valve-closed { background: rgba(239, 68, 68, 0.15); color: var(--danger); border: 1px solid rgba(239, 68, 68, 0.3); }
    .badge-valve-open { background: rgba(16, 185, 129, 0.2); color: var(--success); border: 1px solid rgba(16, 185, 129, 0.4); animation: pulse-border 1.2s infinite; }
    .badge-alarm { background: rgba(239, 68, 68, 0.25); color: #ff5252; border: 1px solid #ef4444; animation: flash-red 0.8s infinite; cursor: pointer; }
    @keyframes pulse-border { 0%, 100% { box-shadow: 0 0 0 0 rgba(16, 185, 129, 0.4); } 50% { box-shadow: 0 0 0 6px rgba(16, 185, 129, 0); } }
    @keyframes flash-red { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }

    /* Main Big Display (SPBU Style) */
    .dispenser-display { background: var(--disp-bg); border: 2px solid #334155; border-radius: 20px; padding: 22px; box-shadow: inset 0 2px 10px rgba(0,0,0,0.8), 0 10px 25px -5px rgba(0,0,0,0.5); display: flex; flex-direction: column; gap: 16px; }
    .disp-row { display: flex; justify-content: space-between; align-items: flex-end; padding-bottom: 12px; border-bottom: 1px dashed #1e293b; }
    .disp-row:last-child { border-bottom: none; padding-bottom: 0; }
    .disp-label { font-size: 0.85rem; font-weight: 800; letter-spacing: 1px; color: var(--text-muted); text-transform: uppercase; }
    .disp-value { font-family: "Courier New", Courier, monospace; font-size: 2.8rem; font-weight: 900; line-height: 1; color: #38bdf8; text-shadow: 0 0 12px rgba(56, 189, 248, 0.5); }
    .disp-cost { color: #f59e0b; text-shadow: 0 0 12px rgba(245, 158, 11, 0.5); font-size: 2.6rem; font-family: "Courier New", Courier, monospace; font-weight: 900; }
    .disp-unit { font-size: 1.1rem; color: #64748b; font-weight: 700; margin-left: 6px; }

    /* Mini Stats */
    .mini-stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(130px, 1fr)); gap: 10px; }
    .stat-box { background: var(--card-bg); border: 1px solid var(--card-border); border-radius: 12px; padding: 12px; text-align: center; }
    .stat-label { font-size: 0.7rem; color: var(--text-muted); text-transform: uppercase; font-weight: 700; }
    .stat-val { font-size: 1.25rem; font-weight: 800; color: #f1f5f9; margin-top: 2px; }

    /* Auto Cut-off Control Station */
    .control-card { background: var(--card-bg); border: 1px solid var(--card-border); border-radius: 16px; padding: 18px; display: flex; flex-direction: column; gap: 14px; }
    .control-header { display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 8px; }
    .control-title { font-size: 0.95rem; font-weight: 700; color: #e2e8f0; display: flex; align-items: center; gap: 8px; }
    .mode-tabs { display: flex; background: #0b1120; padding: 4px; border-radius: 10px; gap: 4px; border: 1px solid #1e293b; }
    .mode-tab { flex: 1; text-align: center; font-size: 0.78rem; font-weight: 700; padding: 6px 10px; border-radius: 8px; cursor: pointer; color: var(--text-muted); transition: all 0.2s; border: none; background: transparent; white-space: nowrap; }
    .mode-tab.active { background: var(--primary); color: #fff; }

    /* Preset Chips */
    .preset-chips { display: flex; flex-wrap: wrap; gap: 8px; }
    .chip { padding: 8px 14px; border-radius: 10px; font-size: 0.82rem; font-weight: 700; background: #0b1120; border: 1px solid #334155; color: #e2e8f0; cursor: pointer; transition: all 0.2s; }
    .chip:hover { border-color: var(--primary); color: var(--primary); }
    .chip.active { background: rgba(14, 165, 233, 0.2); border-color: var(--primary); color: #38bdf8; }

    /* Target Input & Action Buttons */
    .target-bar { display: flex; gap: 10px; align-items: center; }
    .target-input { flex: 1; background: #0b1120; border: 1px solid #334155; border-radius: 10px; padding: 10px 14px; font-size: 1.1rem; font-weight: 700; color: #fff; outline: none; }
    .target-input:focus { border-color: var(--primary); }
    .btn-action-group { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .btn-start { background: linear-gradient(135deg, #10b981, #059669); color: #fff; border: none; border-radius: 12px; padding: 14px; font-size: 1rem; font-weight: 800; cursor: pointer; transition: all 0.2s; box-shadow: 0 4px 12px rgba(16, 185, 129, 0.3); }
    .btn-start:hover { opacity: 0.9; transform: translateY(-1px); }
    .btn-start:disabled { opacity: 0.5; cursor: not-allowed; }
    .btn-stop { background: linear-gradient(135deg, #ef4444, #dc2626); color: #fff; border: none; border-radius: 12px; padding: 14px; font-size: 1rem; font-weight: 800; cursor: pointer; transition: all 0.2s; box-shadow: 0 4px 12px rgba(239, 68, 68, 0.3); }
    .btn-stop:hover { opacity: 0.9; transform: translateY(-1px); }
    .btn-secondary { background: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 10px; padding: 9px 13px; font-size: 0.82rem; font-weight: 700; cursor: pointer; transition: all 0.2s; }
    .btn-secondary:hover { background: #334155; color: #fff; }

    /* Table & Tabs */
    .tabs-header { display: flex; gap: 8px; border-bottom: 1px solid var(--card-border); padding-bottom: 8px; margin-top: 6px; overflow-x: auto; }
    .tab-btn { background: transparent; border: none; font-size: 0.85rem; font-weight: 700; color: var(--text-muted); cursor: pointer; padding: 6px 12px; border-radius: 8px; white-space: nowrap; }
    .tab-btn.active { color: var(--primary); background: rgba(14, 165, 233, 0.1); }
    .tab-content { display: none; }
    .tab-content.active { display: block; }
    table { width: 100%; border-collapse: collapse; font-size: 0.78rem; text-align: left; margin-top: 10px; }
    th { padding: 9px 10px; background: #0b1120; color: var(--text-muted); font-weight: 700; border-bottom: 1px solid var(--card-border); }
    td { padding: 9px 10px; border-bottom: 1px solid #1e293b; }
    .form-group { display: flex; flex-direction: column; gap: 5px; margin-bottom: 12px; }
    .form-group label { font-size: 0.75rem; font-weight: 700; color: var(--text-muted); text-transform: uppercase; }
    .form-input { background: #0b1120; border: 1px solid #334155; border-radius: 8px; padding: 8px 12px; font-size: 0.85rem; color: #fff; outline: none; }
    .form-input:focus { border-color: var(--primary); }

    /* Modal Struk SPBU */
    .modal-overlay { display: none; position: fixed; top: 0; left: 0; width: 100vw; height: 100vh; background: rgba(0,0,0,0.75); z-index: 999; justify-content: center; align-items: center; padding: 15px; }
    .receipt-box { background: #fff; color: #111; border-radius: 10px; padding: 22px; width: 100%; max-width: 380px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); font-family: monospace; font-size: 13px; line-height: 1.4; }
    .receipt-header { text-align: center; border-bottom: 1px dashed #444; padding-bottom: 10px; margin-bottom: 10px; }
    .receipt-header h3 { font-size: 16px; margin-bottom: 2px; }
    .receipt-row { display: flex; justify-content: space-between; margin-bottom: 4px; }
    .receipt-divider { border-top: 1px dashed #444; margin: 8px 0; }
    .receipt-actions { display: flex; gap: 8px; margin-top: 15px; }

    @media print {
      body * { visibility: hidden; }
      .receipt-box, .receipt-box * { visibility: visible; }
      .receipt-box { position: absolute; left: 0; top: 0; width: 100%; max-width: 320px; box-shadow: none; padding: 5px; }
      .receipt-actions { display: none !important; }
    }
    
    footer { text-align: center; font-size: 0.75rem; color: #64748b; padding: 16px 0; }
  </style>
</head>
<body>
  <div class="container">
    <!-- Topbar -->
    <header>
      <div class="brand">
        <div class="brand-icon">⛽</div>
        <div class="brand-text">
          <h1>Universal Flow Meter</h1>
          <span class="fluid-badge" id="topFluidBadge">Solar / Diesel</span>
        </div>
      </div>
      <div class="header-badges">
        <div class="badge badge-valve-closed" id="valveStatusBadge">
          <span style="font-size:10px;">●</span> <span id="valveStatusText">VALVE TERTUTUP</span>
        </div>
        <div class="badge badge-alarm" id="alarmBadge" style="display:none;" onclick="clearAlerts()">
          ⚠️ <span id="alarmText">ALARM</span> [Klik Reset]
        </div>
        <div class="badge" style="background:#0b1120;border:1px solid #334155;color:#94a3b8;" id="clockBadge">
          🕒 <span id="clockText">Memuat jam...</span>
        </div>
        <div class="badge" style="background:#0b1120;border:1px solid #334155;color:#94a3b8;" id="wifiBadge">
          📶 <span id="wifiStatusText">AP: 192.168.4.1</span>
        </div>
      </div>
    </header>

    <!-- SPBU Style Big Display -->
    <div class="dispenser-display">
      <div class="disp-row">
        <div>
          <div class="disp-label">VOLUME PENGISIAN SAAT INI</div>
          <div style="font-size:0.75rem;color:#64748b;" id="dispUnitRate">DEBIT REALTIME</div>
        </div>
        <div style="text-align:right;">
          <span class="disp-value" id="batchVolume">0.000</span>
          <span class="disp-unit" id="batchUnitLabel">L</span>
        </div>
      </div>

      <div class="disp-row">
        <div>
          <div class="disp-label">TOTAL HARGA</div>
          <div style="font-size:0.75rem;color:#64748b;" id="unitPriceLabel">@ Rp 6.800 / L</div>
        </div>
        <div style="text-align:right;">
          <span class="disp-cost" id="batchCost">Rp 0</span>
        </div>
      </div>
    </div>

    <!-- Mini Metrics Grid -->
    <div class="mini-stats">
      <div class="stat-box">
        <div class="stat-label">DEBIT ALIRAN</div>
        <div class="stat-val" style="color:#38bdf8;" id="flowRate">0.00 <span style="font-size:0.7rem;color:#64748b;">L/m</span></div>
      </div>
      <div class="stat-box">
        <div class="stat-label">KONSUMSI PER JAM</div>
        <div class="stat-val" style="color:#a855f7;" id="flowRateHour">0.0 <span style="font-size:0.7rem;color:#64748b;">L/h</span></div>
      </div>
      <div class="stat-box">
        <div class="stat-label">ODOMETER TOTAL</div>
        <div class="stat-val" style="color:#10b981;" id="totalVolume">0.00 <span style="font-size:0.7rem;color:#64748b;">L</span></div>
      </div>
      <div class="stat-box">
        <div class="stat-label">KOREKSI LEAD CUT-OFF</div>
        <div class="stat-val" style="color:#f59e0b;" id="overshootDisplay">20 <span style="font-size:0.7rem;color:#64748b;">mL</span></div>
      </div>
    </div>

    <!-- Auto Cut-off Control Station -->
    <div class="control-card">
      <div class="control-header">
        <div class="control-title">
          <span>🎯</span> Stasiun Kontrol Pengisian & Auto-Cutoff
        </div>
        <div class="mode-tabs">
          <button class="mode-tab active" onclick="setMode('volume', this)">Preset Liter</button>
          <button class="mode-tab" onclick="setMode('amount', this)">Preset Rupiah</button>
          <button class="mode-tab" onclick="setMode('manual', this)">Manual / Bebas</button>
        </div>
      </div>

      <!-- Quick Preset Chips -->
      <div class="preset-chips" id="chipsContainer">
        <!-- Injected via JS -->
      </div>

      <!-- Target input -->
      <div class="target-bar" id="targetInputGroup">
        <input type="number" step="0.1" class="target-input" id="targetInput" placeholder="Target Volume..." value="5.0" />
        <span style="font-weight:700;color:var(--text-muted);" id="targetUnitDisplay">Liter</span>
      </div>

      <!-- Action Buttons -->
      <div class="btn-action-group">
        <button class="btn-start" id="btnStart" onclick="startDispense()">
          ▶ MULAI PENGISIAN
        </button>
        <button class="btn-stop" id="btnStop" onclick="stopDispense()">
          ⏹ STOP / SELESAI
        </button>
      </div>

      <div style="display:flex;gap:8px;justify-content:flex-end;margin-top:4px;">
        <button class="btn-secondary" onclick="resetBatch()">
          🔄 Nolkan Pengisian (Reset Batch)
        </button>
      </div>
    </div>

    <!-- Multi Tab Section: Riwayat Transaksi | Kalibrasi & Profil | WiFi & IoT -->
    <div class="control-card">
      <div class="tabs-header">
        <button class="tab-btn active" onclick="switchTab('historyTab', this)">📜 Riwayat Transaksi (50)</button>
        <button class="tab-btn" onclick="switchTab('profileTab', this)">⚙️ Profil Cairan & Kalibrasi</button>
        <button class="tab-btn" onclick="switchTab('bejanaTab', this)">🧪 Wizard Bejana Ukur</button>
        <button class="tab-btn" onclick="switchTab('wifiTab', this)">📶 WiFi, Jam & IoT Hub</button>
      </div>

      <!-- TAB 1: Riwayat Transaksi -->
      <div class="tab-content active" id="historyTab">
        <div style="display:flex;justify-content:space-between;align-items:center;margin:10px 0;flex-wrap:wrap;gap:8px;">
          <span style="font-size:0.8rem;color:var(--text-muted);">Riwayat pengisian otomatis tersimpan di Flash NVS</span>
          <div style="display:flex;gap:8px;">
            <a href="/api/history/export" class="btn-secondary" style="text-decoration:none;display:inline-flex;align-items:center;gap:6px;">
              📥 Download CSV (Excel)
            </a>
            <button class="btn-secondary" onclick="clearHistory()" style="color:#ef4444;">
              🗑️ Hapus
            </button>
          </div>
        </div>
        <div style="overflow-x:auto;">
          <table>
            <thead>
              <tr>
                <th>#ID</th>
                <th>WAKTU</th>
                <th>CAIRAN</th>
                <th>VOLUME</th>
                <th>TOTAL HARGA</th>
                <th>MODE</th>
                <th>STRUK</th>
              </tr>
            </thead>
            <tbody id="historyTableBody">
              <tr><td colspan="7" style="text-align:center;color:#64748b;padding:20px;">Memuat riwayat transaksi...</td></tr>
            </tbody>
          </table>
        </div>
      </div>

      <!-- TAB 2: Profil Cairan & Kalibrasi -->
      <div class="tab-content" id="profileTab">
        <div style="padding:10px 0;">
          <div class="form-group">
            <label>Katalog Preset Bahan Bakar / Cairan Cepat:</label>
            <select class="form-input" onchange="applyFluidPreset(this.value)">
              <option value="">-- Pilih Preset Bahan Bakar / Cairan --</option>
              <option value="biosolar">⛽ Bio Solar Subsidi (Rp 6.800 / L)</option>
              <option value="dexlite">⛽ Dexlite (Rp 13.050 / L)</option>
              <option value="pertdex">⛽ Pertamina Dex (Rp 13.700 / L)</option>
              <option value="pertalite">⛽ Pertalite RON 90 (Rp 10.000 / L)</option>
              <option value="pertamax">⛽ Pertamax RON 92 (Rp 12.100 / L)</option>
              <option value="pertamax_green">⛽ Pertamax Green 95 (Rp 13.150 / L)</option>
              <option value="pertamax_turbo">⛽ Pertamax Turbo RON 98 (Rp 13.250 / L)</option>
              <option value="minyak_goreng">🛢️ Minyak Goreng Curah (Rp 15.500 / L)</option>
              <option value="air_ro">💧 Air Minum RO / Bersih (Rp 50 / L)</option>
            </select>
          </div>

          <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;">
            <div class="form-group">
              <label>Nama Cairan / Fluida:</label>
              <input type="text" class="form-input" id="cfgFluidName" value="Solar / Diesel" />
            </div>
            <div class="form-group">
              <label>Satuan Volume:</label>
              <input type="text" class="form-input" id="cfgUnitName" value="Liter" />
            </div>
          </div>

          <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;">
            <div class="form-group">
              <label>Mata Uang:</label>
              <input type="text" class="form-input" id="cfgCurrency" value="Rp" />
            </div>
            <div class="form-group">
              <label>Harga per Satuan (Rp):</label>
              <input type="number" step="100" class="form-input" id="cfgPrice" value="6800" />
            </div>
          </div>

          <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;">
            <div class="form-group">
              <label>K-Factor Sensor (pulsa/L):</label>
              <input type="number" step="0.1" class="form-input" id="cfgKFactor" value="400.0" />
            </div>
            <div class="form-group">
              <label>Kompensasi Lead Inching / Cut-off (Liter):</label>
              <input type="number" step="0.005" class="form-input" id="cfgOvershoot" value="0.02" placeholder="0.02 = 20mL" />
            </div>
          </div>

          <div class="form-group">
            <label>PIN Admin (Keamanan):</label>
            <input type="password" class="form-input" id="cfgAdminPin" placeholder="Masukkan PIN (Default: 1234)" />
          </div>

          <button class="btn-start" style="width:100%;padding:12px;" onclick="saveProfileSettings()">
            💾 Simpan Pengaturan Profil ke ESP32
          </button>

          <hr style="border:none;border-top:1px dashed #334155;margin:18px 0;" />

          <div style="display:flex;justify-content:space-between;align-items:center;">
            <div>
              <div style="font-size:0.85rem;font-weight:700;color:#ef4444;">Nolkan Odometer Seumur Hidup</div>
              <div style="font-size:0.75rem;color:var(--text-muted);">Reset total akumulasi literan permanen (memerlukan PIN).</div>
            </div>
            <button class="btn-secondary" style="color:#ef4444;border-color:rgba(239,68,68,0.4);" onclick="resetTotalLifetime()">
              ⚠️ Reset Total
            </button>
          </div>
        </div>
      </div>

      <!-- TAB 3: Wizard Bejana Ukur -->
      <div class="tab-content" id="bejanaTab">
        <div style="padding:10px 0;">
          <h4 style="font-size:0.95rem;color:#38bdf8;margin-bottom:6px;">🧪 Wizard Kalibrasi Bejana Ukur Presisi</h4>
          <p style="font-size:0.8rem;color:var(--text-muted);line-height:1.4;margin-bottom:14px;">
            Gunakan bejana ukur standar tera (misal 10 Liter atau 20 Liter). Sistem akan menghitung jumlah pulsa sensor aktual dan secara otomatis mengkalkulasi K-Factor baru yang presisi tanpa rumus manual.
          </p>

          <div style="background:#0b1120;padding:14px;border-radius:12px;border:1px solid #1e293b;margin-bottom:14px;">
            <div style="font-size:0.8rem;font-weight:700;color:#e2e8f0;margin-bottom:8px;">Langkah-langkah Kalibrasi:</div>
            <ol style="font-size:0.78rem;color:var(--text-muted);padding-left:18px;line-height:1.5;">
              <li>Siapkan bejana ukur kosong (misal 10 Liter).</li>
              <li>Klik tombol <b>[Nolkan Pengisian (Reset Batch)]</b> agar pulsa kembali ke 0.</li>
              <li>Alirkan cairan menggunakan mode <b>Manual</b> hingga permukaan cairan tepat di garis tera bejana.</li>
              <li>Hentikan aliran, lalu pilih ukuran bejana di bawah dan klik <b>[Terapkan Kalibrasi Otomatis]</b>.</li>
            </ol>
          </div>

          <div class="form-group">
            <label>Ukuran Bejana Ukur Tera:</label>
            <select class="form-input" id="bejanaSize">
              <option value="10.0">Bejana Ukur 10 Liter</option>
              <option value="20.0">Bejana Ukur 20 Liter</option>
              <option value="5.0">Bejana Ukur 5 Liter</option>
              <option value="50.0">Bejana / Drum 50 Liter</option>
            </select>
          </div>

          <div class="form-group">
            <label>PIN Admin untuk Kalibrasi:</label>
            <input type="password" class="form-input" id="bejanaPin" placeholder="PIN Admin (Default: 1234)" />
          </div>

          <button class="btn-start" style="width:100%;padding:12px;background:linear-gradient(135deg, #0ea5e9, #2563eb);" onclick="runBejanaCalibration()">
            🎯 Terapkan Kalibrasi Bejana (Hitung K-Factor Baru)
          </button>
        </div>
      </div>

      <!-- TAB 4: WiFi, Jam & IoT Hub -->
      <div class="tab-content" id="wifiTab">
        <div style="padding:10px 0;">
          <h4 style="font-size:0.95rem;color:#38bdf8;margin-bottom:6px;">📶 Koneksi WiFi Stasiun (STA Mode)</h4>
          <p style="font-size:0.8rem;color:var(--text-muted);margin-bottom:14px;">
            Sambungkan ESP32 ke WiFi rumah/kantor untuk sinkronisasi jam NTP otomatis dan integrasi ke server Omni Agent Hub.
          </p>

          <div class="form-group">
            <label>SSID WiFi Rumah / Kantor:</label>
            <input type="text" class="form-input" id="staSsid" placeholder="Nama WiFi (SSID)" />
          </div>
          <div class="form-group">
            <label>Password WiFi:</label>
            <input type="password" class="form-input" id="staPass" placeholder="Password WiFi" />
          </div>

          <h4 style="font-size:0.95rem;color:#f59e0b;margin-top:16px;margin-bottom:6px;">🌐 Webhook Push Omni Agent Hub (IoT Integration)</h4>
          <div class="form-group">
            <label>URL Webhook Server (Dipanggil tiap selesai pengisian):</label>
            <input type="text" class="form-input" id="cfgWebhookUrl" placeholder="http://192.168.100.184:3800/api/flowmeter/transaction" />
          </div>

          <div class="form-group">
            <label>PIN Admin:</label>
            <input type="password" class="form-input" id="staPin" placeholder="PIN Admin (Default: 1234)" />
          </div>

          <div style="display:flex;gap:10px;flex-wrap:wrap;">
            <button class="btn-start" style="flex:1;padding:12px;" onclick="saveWifiSettings()">
              📶 Sambungkan WiFi & Simpan Webhook
            </button>
            <button class="btn-secondary" onclick="syncBrowserTime()">
              🕒 Sinkronkan Jam Ponsel ke ESP32
            </button>
          </div>
        </div>
      </div>
    </div>

    <!-- Modal Cetak Struk SPBU -->
    <div class="modal-overlay" id="receiptModal">
      <div class="receipt-box" id="printableReceipt">
        <div class="receipt-header">
          <h3>⛽ DIGITAL FLOW METER</h3>
          <p>STASIUN DISPENSER BAHAN BAKAR</p>
          <p style="font-size:11px;color:#666;">STRUK BUKTI PENGISIAN RESMI</p>
        </div>
        <div class="receipt-row">
          <span>No. Transaksi</span>
          <strong id="rcptId">#0001</strong>
        </div>
        <div class="receipt-row">
          <span>Waktu</span>
          <span id="rcptTime">2026-11-12 16:45:00</span>
        </div>
        <div class="receipt-row">
          <span>Plat Nomor</span>
          <input type="text" id="rcptPlate" value="BG 1234 XX" style="font-family:monospace;font-size:12px;border:1px solid #ccc;padding:2px 4px;width:100px;text-align:right;" />
        </div>
        <div class="receipt-divider"></div>
        <div class="receipt-row">
          <span id="rcptFluid">Solar / Diesel</span>
          <span id="rcptMode">Preset Liter</span>
        </div>
        <div class="receipt-row">
          <span>Volume</span>
          <strong><span id="rcptVolume">5.000</span> <span id="rcptUnit">L</span></strong>
        </div>
        <div class="receipt-row">
          <span>Harga / Satuan</span>
          <span id="rcptPrice">Rp 6.800</span>
        </div>
        <div class="receipt-divider"></div>
        <div class="receipt-row" style="font-size:16px;">
          <strong>TOTAL BAYAR</strong>
          <strong><span id="rcptCost">Rp 34.000</span></strong>
        </div>
        <div class="receipt-divider"></div>
        <div style="text-align:center;font-size:11px;color:#555;margin-top:8px;">
          <p>Terima Kasih & Selamat Jalan</p>
          <p style="font-size:10px;margin-top:2px;">Sistem Literan Presisi ESP32-S3 + AICHI</p>
        </div>
        <div class="receipt-actions">
          <button class="btn-start" style="flex:1;padding:8px;" onclick="window.print()">🖨️ Cetak Struk</button>
          <button class="btn-secondary" style="padding:8px 14px;" onclick="closeReceiptModal()">Tutup</button>
        </div>
      </div>
    </div>

    <footer>
      Smart Digital Flow Meter & Dispenser Controller &bull; ESP32-S3 + AICHI Sensor &bull; Universal Liquid Ready
    </footer>
  </div>

  <script>
    let activeMode = "volume";
    let cachedHistory = [];
    let initialTimeSynced = false;

    const PRESET_VOLUMES = [1, 2, 5, 10, 20, 50];
    const PRESET_AMOUNTS = [20000, 50000, 100000, 200000, 500000];

    const FLUID_PRESETS = {
      biosolar: { name: "Bio Solar Subsidi", price: 6800, k: 400.0 },
      dexlite: { name: "Dexlite", price: 13050, k: 400.0 },
      pertdex: { name: "Pertamina Dex", price: 13700, k: 400.0 },
      pertalite: { name: "Pertalite RON 90", price: 10000, k: 400.0 },
      pertamax: { name: "Pertamax RON 92", price: 12100, k: 400.0 },
      pertamax_green: { name: "Pertamax Green 95", price: 13150, k: 400.0 },
      pertamax_turbo: { name: "Pertamax Turbo RON 98", price: 13250, k: 400.0 },
      minyak_goreng: { name: "Minyak Goreng Curah", price: 15500, k: 398.0 },
      air_ro: { name: "Air Bersih / RO", price: 50, k: 400.0 }
    };

    function renderChips() {
      const c = document.getElementById("chipsContainer");
      c.innerHTML = "";
      const list = activeMode === "volume" ? PRESET_VOLUMES : activeMode === "amount" ? PRESET_AMOUNTS : [];
      list.forEach(val => {
        const chip = document.createElement("button");
        chip.className = "chip";
        chip.textContent = activeMode === "volume" ? `${val} L` : `Rp ${val.toLocaleString('id-ID')}`;
        chip.onclick = () => {
          document.querySelectorAll(".chip").forEach(x => x.classList.remove("active"));
          chip.classList.add("active");
          document.getElementById("targetInput").value = val;
        };
        c.appendChild(chip);
      });
    }

    function setMode(mode, btn) {
      activeMode = mode;
      document.querySelectorAll(".mode-tab").forEach(t => t.classList.remove("active"));
      if (btn) btn.classList.add("active");

      const inputGroup = document.getElementById("targetInputGroup");
      const unitDisp = document.getElementById("targetUnitDisplay");

      if (mode === "manual") {
        inputGroup.style.display = "none";
        document.getElementById("chipsContainer").style.display = "none";
      } else {
        inputGroup.style.display = "flex";
        document.getElementById("chipsContainer").style.display = "flex";
        unitDisp.textContent = mode === "volume" ? "Liter" : "Rupiah (Rp)";
        document.getElementById("targetInput").value = mode === "volume" ? "5.0" : "50000";
        renderChips();
      }
    }

    function switchTab(tabId, btn) {
      document.querySelectorAll(".tab-content").forEach(c => c.classList.remove("active"));
      document.querySelectorAll(".tab-btn").forEach(b => b.classList.remove("active"));
      document.getElementById(tabId).classList.add("active");
      if (btn) btn.classList.add("active");
      if (tabId === "historyTab") fetchHistory();
    }

    function applyFluidPreset(key) {
      if (!key || !FLUID_PRESETS[key]) return;
      const p = FLUID_PRESETS[key];
      document.getElementById("cfgFluidName").value = p.name;
      document.getElementById("cfgPrice").value = p.price;
      document.getElementById("cfgKFactor").value = p.k;
    }

    // Auto-sync browser phone time to ESP32 silently on first load
    function autoSyncBrowserTime() {
      if (initialTimeSynced) return;
      initialTimeSynced = true;
      const epochSec = Math.floor(Date.now() / 1000);
      fetch("/api/time/sync", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ epoch: epochSec })
      }).catch(() => {});
    }

    async function syncBrowserTime() {
      const epochSec = Math.floor(Date.now() / 1000);
      try {
        const r = await fetch("/api/time/sync", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ epoch: epochSec })
        });
        const res = await r.json();
        alert("Jam RTC ESP32 berhasil disinkronkan ke: " + res.time);
      } catch (e) {
        alert("Gagal menyinkronkan jam.");
      }
    }

    async function fetchData() {
      try {
        const res = await fetch("/api/data");
        const d = await res.json();

        autoSyncBrowserTime();

        document.getElementById("topFluidBadge").textContent = d.fluid_name;
        document.getElementById("batchVolume").textContent = Number(d.batch_liters).toFixed(3);
        document.getElementById("batchUnitLabel").textContent = d.unit_name === "Liter" ? "L" : d.unit_name;
        document.getElementById("batchCost").textContent = `${d.currency} ${Math.round(d.batch_cost).toLocaleString('id-ID')}`;
        document.getElementById("unitPriceLabel").textContent = `@ ${d.currency} ${Math.round(d.price_per_liter).toLocaleString('id-ID')} / ${d.unit_name}`;

        document.getElementById("flowRate").innerHTML = `${Number(d.flow_rate_lpm).toFixed(2)} <span style="font-size:0.7rem;color:#64748b;">${d.unit_name}/m</span>`;
        document.getElementById("flowRateHour").innerHTML = `${(Number(d.flow_rate_lpm) * 60).toFixed(1)} <span style="font-size:0.7rem;color:#64748b;">${d.unit_name}/h</span>`;
        document.getElementById("totalVolume").innerHTML = `${Number(d.total_liters).toFixed(2)} <span style="font-size:0.7rem;color:#64748b;">${d.unit_name}</span>`;
        document.getElementById("overshootDisplay").innerHTML = `${Math.round((d.overshoot_liters || 0.02) * 1000)} <span style="font-size:0.7rem;color:#64748b;">mL</span>`;

        // Clock display
        document.getElementById("clockText").textContent = d.system_time || "Standby";

        // Valve badge
        const badge = document.getElementById("valveStatusBadge");
        const badgeTxt = document.getElementById("valveStatusText");
        if (d.valve_open) {
          badge.className = "badge badge-valve-open";
          badgeTxt.textContent = "VALVE TERBUKA (MENGALIR)";
          document.getElementById("btnStart").disabled = true;
          document.getElementById("btnStop").disabled = false;
        } else {
          badge.className = "badge badge-valve-closed";
          badgeTxt.textContent = "VALVE TERTUTUP";
          document.getElementById("btnStart").disabled = false;
          document.getElementById("btnStop").disabled = true;
        }

        // Alarm badge
        const alarmEl = document.getElementById("alarmBadge");
        const alarmTxt = document.getElementById("alarmText");
        if (d.dry_run_alarm) {
          alarmEl.style.display = "flex";
          alarmTxt.textContent = "DRY-RUN! POMPA KOSONG";
        } else if (d.leakage_alert) {
          alarmEl.style.display = "flex";
          alarmTxt.textContent = "REMBES / BOCOR TERDETEKSI";
        } else {
          alarmEl.style.display = "none";
        }

        // WiFi badge
        if (d.sta_connected && d.sta_ip) {
          document.getElementById("wifiStatusText").textContent = `LAN: ${d.sta_ip}`;
        } else {
          document.getElementById("wifiStatusText").textContent = `AP: ${d.ap_ip}`;
        }

      } catch (err) {
        console.error("Fetch data error:", err);
      }
    }

    async function startDispense() {
      const targetVal = parseFloat(document.getElementById("targetInput").value) || 0;
      if (activeMode !== "manual" && targetVal <= 0) {
        alert("Target pengisian harus lebih besar dari 0!");
        return;
      }
      try {
        await fetch("/api/dispense/start", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ mode: activeMode, target: targetVal })
        });
        fetchData();
      } catch (e) {
        alert("Gagal memulai pengisian.");
      }
    }

    async function stopDispense() {
      try {
        await fetch("/api/dispense/stop", { method: "POST" });
        fetchData();
        setTimeout(fetchHistory, 800);
      } catch (e) {
        alert("Gagal menghentikan pengisian.");
      }
    }

    async function resetBatch() {
      if (confirm("Nolkan volume pengisian saat ini (Reset Batch)?")) {
        await fetch("/api/reset_batch", { method: "POST" });
        fetchData();
      }
    }

    async function clearAlerts() {
      await fetch("/api/alerts/clear", { method: "POST" });
      fetchData();
    }

    async function fetchHistory() {
      try {
        const res = await fetch("/api/history");
        const records = await res.json();
        cachedHistory = records;
        const tbody = document.getElementById("historyTableBody");
        if (!records || records.length === 0) {
          tbody.innerHTML = `<tr><td colspan="7" style="text-align:center;color:#64748b;padding:20px;">Belum ada riwayat transaksi pengisian.</td></tr>`;
          return;
        }
        let html = "";
        records.forEach((r, idx) => {
          html += `<tr>
            <td style="font-weight:700;">#${r.id}</td>
            <td style="color:#94a3b8;font-size:0.72rem;">${r.time_str || ("Uptime " + r.uptime_sec + "s")}</td>
            <td>${r.fluid}</td>
            <td style="font-weight:700;color:#38bdf8;">${Number(r.volume).toFixed(3)} ${r.unit}</td>
            <td style="font-weight:700;color:#f59e0b;">${r.currency} ${Math.round(r.cost).toLocaleString('id-ID')}</td>
            <td><span style="font-size:0.7rem;padding:2px 6px;border-radius:4px;background:#1e293b;color:#cbd5e1;">${r.mode}</span></td>
            <td><button class="btn-secondary" style="padding:4px 8px;font-size:0.75rem;" onclick="openReceiptModal(${idx})">🖨️ Struk</button></td>
          </tr>`;
        });
        tbody.innerHTML = html;
      } catch (e) {
        console.error("History fetch error:", e);
      }
    }

    function openReceiptModal(idx) {
      const r = cachedHistory[idx];
      if (!r) return;
      document.getElementById("rcptId").textContent = `#${String(r.id).padStart(4, '0')}`;
      document.getElementById("rcptTime").textContent = r.time_str || "WIB";
      document.getElementById("rcptFluid").textContent = r.fluid;
      document.getElementById("rcptMode").textContent = r.mode === "preset_volume" ? "Preset Liter" : r.mode === "preset_amount" ? "Preset Rupiah" : "Manual";
      document.getElementById("rcptVolume").textContent = Number(r.volume).toFixed(3);
      document.getElementById("rcptUnit").textContent = r.unit;
      const pricePerL = r.volume > 0 ? Math.round(r.cost / r.volume) : 6800;
      document.getElementById("rcptPrice").textContent = `${r.currency} ${pricePerL.toLocaleString('id-ID')}`;
      document.getElementById("rcptCost").textContent = `${r.currency} ${Math.round(r.cost).toLocaleString('id-ID')}`;

      document.getElementById("receiptModal").style.display = "flex";
    }

    function closeReceiptModal() {
      document.getElementById("receiptModal").style.display = "none";
    }

    async function clearHistory() {
      const pin = prompt("Masukkan PIN Admin untuk menghapus seluruh riwayat transaksi:");
      if (!pin) return;
      const res = await fetch("/api/history/clear", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ pin: pin })
      });
      const data = await res.json();
      if (data.status === "ok") {
        alert("Riwayat transaksi berhasil dikosongkan.");
        fetchHistory();
      } else {
        alert(data.error || "Gagal menghapus riwayat.");
      }
    }

    async function runBejanaCalibration() {
      const bejanaVol = parseFloat(document.getElementById("bejanaSize").value) || 10.0;
      const pin = document.getElementById("bejanaPin").value;
      if (!pin) {
        alert("Silakan masukkan PIN Admin terlebih dahulu!");
        return;
      }
      try {
        const res = await fetch("/api/calibrate/bejana", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ target_volume: bejanaVol, pin: pin })
        });
        const data = await res.json();
        if (data.status === "ok") {
          alert(`Kalibrasi Bejana Berhasil!\nJumlah Pulsa: ${data.pulses}\nK-Factor Baru: ${Number(data.k_factor).toFixed(3)} pulsa/L`);
          document.getElementById("cfgKFactor").value = Number(data.k_factor).toFixed(3);
          fetchData();
        } else {
          alert(data.error || "Gagal melakukan kalibrasi bejana.");
        }
      } catch (e) {
        alert("Gagal menghubungi alat untuk kalibrasi.");
      }
    }

    async function saveProfileSettings() {
      const pin = document.getElementById("cfgAdminPin").value;
      if (!pin) {
        alert("Silakan masukkan PIN Admin untuk menyimpan!");
        return;
      }
      const payload = {
        pin: pin,
        fluid_name: document.getElementById("cfgFluidName").value,
        unit_name: document.getElementById("cfgUnitName").value,
        currency: document.getElementById("cfgCurrency").value,
        price: parseFloat(document.getElementById("cfgPrice").value),
        k_factor: parseFloat(document.getElementById("cfgKFactor").value),
        overshoot: parseFloat(document.getElementById("cfgOvershoot").value)
      };
      try {
        const res = await fetch("/api/settings", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(payload)
        });
        const data = await res.json();
        if (data.status === "success") {
          alert("Pengaturan profil cairan berhasil disimpan ke NVS ESP32!");
          document.getElementById("cfgAdminPin").value = "";
          fetchData();
        } else {
          alert(data.error || "Gagal menyimpan pengaturan.");
        }
      } catch (e) {
        alert("Gagal menghubungi alat.");
      }
    }

    async function saveWifiSettings() {
      const pin = document.getElementById("staPin").value;
      if (!pin) {
        alert("Masukkan PIN Admin!");
        return;
      }
      const payload = {
        pin: pin,
        ssid: document.getElementById("staSsid").value,
        password: document.getElementById("staPass").value
      };
      // Simpan juga webhook jika ada
      const webhook = document.getElementById("cfgWebhookUrl").value;
      if (webhook) {
        await fetch("/api/settings", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ pin: pin, webhook: webhook })
        }).catch(() => {});
      }
      try {
        const res = await fetch("/api/wifi/settings", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(payload)
        });
        const data = await res.json();
        alert(data.message || "Pengaturan tersimpan.");
        fetchData();
      } catch (e) {
        alert("Gagal menyimpan WiFi.");
      }
    }

    async function resetTotalLifetime() {
      const pin = prompt("PERINGATAN: Total akumulasi seumur hidup akan dinolkan!\nMasukkan PIN Admin:");
      if (!pin) return;
      const res = await fetch("/api/reset_total", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ pin: pin })
      });
      const data = await res.json();
      if (data.status === "success") {
        alert("Total seumur hidup berhasil dinolkan!");
        fetchData();
      } else {
        alert(data.error || "Gagal menolkan total seumur hidup.");
      }
    }

    // Init
    setMode("volume");
    fetchData();
    setInterval(fetchData, 400);
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
