#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Universal Liquid Flow Meter | AICHI Sensor</title>
  <style>
    :root {
      --bg: #0b1120;
      --card-bg: #1e293b;
      --card-border: #334155;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --primary: #f59e0b;
      --primary-hover: #d97706;
      --success: #10b981;
      --danger: #ef4444;
      --accent: #38bdf8;
      --disp-bg: #020617;
    }
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    }
    body {
      background-color: var(--bg);
      color: var(--text);
      padding: 14px;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .container {
      width: 100%;
      max-width: 760px;
    }
    header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px 18px;
      background: var(--card-bg);
      border-radius: 14px;
      border: 1px solid var(--card-border);
      margin-bottom: 14px;
    }
    .brand {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    .brand-icon {
      width: 42px;
      height: 42px;
      background: linear-gradient(135deg, #0ea5e9, #2563eb);
      border-radius: 12px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 22px;
      box-shadow: 0 4px 12px rgba(14, 165, 233, 0.3);
    }
    .brand-text h1 {
      font-size: 1.15rem;
      font-weight: 700;
      letter-spacing: 0.5px;
    }
    .fluid-badge {
      display: inline-block;
      font-size: 0.75rem;
      font-weight: 700;
      padding: 2px 8px;
      border-radius: 6px;
      background: rgba(245, 158, 11, 0.2);
      color: #f59e0b;
      border: 1px solid rgba(245, 158, 11, 0.4);
      margin-top: 2px;
    }
    .status-badge {
      padding: 6px 14px;
      border-radius: 9999px;
      font-size: 0.75rem;
      font-weight: 700;
      display: flex;
      align-items: center;
      gap: 6px;
      background: rgba(16, 185, 129, 0.15);
      color: var(--success);
      border: 1px solid rgba(16, 185, 129, 0.3);
    }
    .status-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--success);
    }
    .status-dot.active {
      animation: pulse 0.8s infinite alternate;
    }
    @keyframes pulse {
      from { transform: scale(0.8); opacity: 0.6; }
      to { transform: scale(1.4); opacity: 1; }
    }

    /* Main Digital Pump Display */
    .pump-display {
      background: var(--disp-bg);
      border: 3px solid #334155;
      border-radius: 18px;
      padding: 22px;
      margin-bottom: 16px;
      box-shadow: inset 0 2px 20px rgba(0,0,0,0.9), 0 8px 30px rgba(0,0,0,0.6);
    }
    .display-row {
      margin-bottom: 16px;
    }
    .display-row:last-child {
      margin-bottom: 0;
    }
    .display-label {
      font-size: 0.8rem;
      font-weight: 700;
      color: #94a3b8;
      letter-spacing: 1px;
      text-transform: uppercase;
      margin-bottom: 5px;
      display: flex;
      justify-content: space-between;
    }
    .display-digits {
      font-family: 'Courier New', Courier, monospace;
      font-weight: 900;
      text-align: right;
      padding: 10px 18px;
      background: #000000;
      border-radius: 8px;
      border: 1px solid #1e293b;
      letter-spacing: 2px;
      overflow-x: auto;
    }
    .digits-liters {
      font-size: 3.2rem;
      color: #38bdf8;
      text-shadow: 0 0 15px rgba(56, 189, 248, 0.45);
    }
    .digits-price {
      font-size: 2.2rem;
      color: #facc15;
      text-shadow: 0 0 15px rgba(250, 204, 21, 0.45);
    }
    .digits-flowrate {
      font-size: 1.8rem;
      color: #4ade80;
      text-shadow: 0 0 15px rgba(74, 222, 128, 0.45);
    }

    /* Quick Action Controls */
    .action-grid {
      display: grid;
      grid-template-columns: 2fr 1fr;
      gap: 10px;
      margin-bottom: 16px;
    }
    .btn {
      padding: 14px 18px;
      font-size: 0.95rem;
      font-weight: 700;
      border: none;
      border-radius: 12px;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      transition: all 0.15s ease;
    }
    .btn:active {
      transform: scale(0.98);
    }
    .btn-reset {
      background: linear-gradient(135deg, #ef4444, #dc2626);
      color: white;
      box-shadow: 0 4px 14px rgba(239, 68, 68, 0.35);
    }
    .btn-reset:hover {
      background: linear-gradient(135deg, #dc2626, #b91c1c);
    }
    .btn-settings {
      background: var(--card-bg);
      color: var(--text);
      border: 1px solid var(--card-border);
    }
    .btn-settings:hover {
      background: #334155;
    }

    /* Metrics Grid */
    .metrics-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
      gap: 10px;
      margin-bottom: 16px;
    }
    .metric-card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 12px;
      padding: 12px 14px;
    }
    .metric-title {
      font-size: 0.72rem;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.5px;
      margin-bottom: 4px;
    }
    .metric-val {
      font-size: 1.25rem;
      font-weight: 700;
      color: var(--text);
    }
    .metric-unit {
      font-size: 0.8rem;
      font-weight: 400;
      color: var(--text-muted);
    }

    /* Card Panels */
    .card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 14px;
      padding: 18px;
      margin-bottom: 16px;
    }
    .card h2 {
      font-size: 1.05rem;
      margin-bottom: 14px;
      display: flex;
      align-items: center;
      gap: 8px;
      color: var(--accent);
    }

    /* Preset Liquid Chips */
    .preset-label {
      font-size: 0.82rem;
      color: var(--text-muted);
      margin-bottom: 8px;
      font-weight: 600;
    }
    .preset-chips {
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      margin-bottom: 16px;
    }
    .chip {
      background: #0f172a;
      border: 1px solid var(--card-border);
      color: var(--text);
      padding: 8px 12px;
      border-radius: 8px;
      font-size: 0.82rem;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 6px;
      transition: all 0.15s ease;
    }
    .chip:hover {
      border-color: var(--accent);
      background: #1e293b;
    }
    .chip.active {
      background: rgba(14, 165, 233, 0.2);
      border-color: var(--accent);
      color: #38bdf8;
      font-weight: 700;
    }

    /* Forms */
    .form-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
    }
    .form-group {
      margin-bottom: 12px;
    }
    .form-group.full {
      grid-column: 1 / -1;
    }
    .form-group label {
      display: block;
      font-size: 0.8rem;
      color: var(--text-muted);
      margin-bottom: 5px;
    }
    .form-input {
      width: 100%;
      padding: 10px 12px;
      background: #0b1120;
      border: 1px solid var(--card-border);
      border-radius: 8px;
      color: white;
      font-size: 0.95rem;
    }
    .form-input:focus {
      outline: none;
      border-color: var(--accent);
    }
    .btn-save {
      background: linear-gradient(135deg, #0ea5e9, #0284c7);
      color: white;
      font-weight: 700;
      width: 100%;
      margin-top: 6px;
      box-shadow: 0 4px 12px rgba(14, 165, 233, 0.3);
    }
    .btn-save:hover {
      background: #0284c7;
    }

    .calib-box {
      background: rgba(14, 165, 233, 0.08);
      border: 1px dashed rgba(14, 165, 233, 0.35);
      padding: 14px;
      border-radius: 10px;
      font-size: 0.82rem;
      color: #bae6fd;
      line-height: 1.45;
      margin-top: 14px;
    }

    footer {
      text-align: center;
      font-size: 0.75rem;
      color: var(--text-muted);
      margin-top: auto;
      padding-top: 14px;
    }

    @media (max-width: 480px) {
      .digits-liters { font-size: 2.2rem; }
      .digits-price { font-size: 1.6rem; }
      .digits-flowrate { font-size: 1.35rem; }
      .action-grid { grid-template-columns: 1fr; }
      .form-grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <div class="brand">
        <div class="brand-icon">&#128167;</div>
        <div class="brand-text">
          <h1 id="topFluidTitle">FLOW METER DIGITAL</h1>
          <span class="fluid-badge" id="topFluidBadge">Solar / Diesel</span>
        </div>
      </div>
      <div class="status-badge" id="flowStatusBadge">
        <span class="status-dot" id="flowDot"></span>
        <span id="flowStatusText">STANDBY</span>
      </div>
    </header>

    <!-- Main Digital Pump Display -->
    <div class="pump-display">
      <div class="display-row">
        <div class="display-label">
          <span id="batchLabel">PENGISIAN SAAT INI (BATCH)</span>
          <span id="batchUnitLabel">LITER</span>
        </div>
        <div class="display-digits digits-liters" id="batchVolume">0.000</div>
      </div>

      <div class="display-row">
        <div class="display-label">
          <span>TOTAL BIAYA (HARGA)</span>
          <span id="unitPriceLabel">@ Rp 6.800/L</span>
        </div>
        <div class="display-digits digits-price" id="batchCost">Rp 0</div>
      </div>

      <div class="display-row">
        <div class="display-label">
          <span>DEBIT ALIRAN</span>
          <span id="flowRateUnitLabel">L / MENIT</span>
        </div>
        <div class="display-digits digits-flowrate" id="flowRate">0.00</div>
      </div>
    </div>

    <!-- Quick Action Controls -->
    <div class="action-grid">
      <button class="btn btn-reset" onclick="resetBatch()">
        <span>&#8635;</span> RESET / NOLKAN PENGISIAN
      </button>
      <button class="btn btn-settings" onclick="toggleSettings()">
        <span>&#9881;</span> PILIH CAIRAN & KALIBRASI
      </button>
    </div>

    <!-- Metrics Cards -->
    <div class="metrics-grid">
      <div class="metric-card">
        <div class="metric-title">Total Akumulasi</div>
        <div class="metric-val" id="totalVolume">0.00 <span class="metric-unit" id="totalUnit">L</span></div>
      </div>
      <div class="metric-card">
        <div class="metric-title">Debit per Jam</div>
        <div class="metric-val" id="flowRateHour">0.0 <span class="metric-unit" id="hourUnit">L/h</span></div>
      </div>
      <div class="metric-card">
        <div class="metric-title">Pulsa Dihitung</div>
        <div class="metric-val" id="batchPulses">0 <span class="metric-unit">pls</span></div>
      </div>
      <div class="metric-card">
        <div class="metric-title">K-Factor Aktif</div>
        <div class="metric-val" id="dispKFactor">400.0 <span class="metric-unit" id="kUnit">pls/L</span></div>
      </div>
    </div>

    <!-- Settings & Fluid Profiles Section -->
    <div class="card" id="settingsCard" style="display: none;">
      <h2>&#9881; Pengaturan Profil Cairan & Kalibrasi</h2>

      <!-- Preset Cairan Cepat -->
      <div class="preset-label">Pilih Profil Jenis Cairan (Rekomendasi Presets):</div>
      <div class="preset-chips">
        <button type="button" class="chip" onclick="applyPreset('solar')">&#9981; Solar / Diesel</button>
        <button type="button" class="chip" onclick="applyPreset('bensin')">&#128692; Bensin / Gas</button>
        <button type="button" class="chip" onclick="applyPreset('minyak_goreng')">&#127859; Minyak Goreng</button>
        <button type="button" class="chip" onclick="applyPreset('oli')">&#128738; Oli Pelumas</button>
        <button type="button" class="chip" onclick="applyPreset('susu')">&#129371; Susu Segar</button>
        <button type="button" class="chip" onclick="applyPreset('minyak_tanah')">&#127982; Minyak Tanah</button>
        <button type="button" class="chip" onclick="applyPreset('air')">&#128167; Air Bersih</button>
        <button type="button" class="chip" onclick="applyPreset('custom')">&#129514; Kustom Cairan</button>
      </div>

      <form id="settingsForm" onsubmit="saveSettings(event)">
        <div class="form-grid">
          <div class="form-group">
            <label for="fluidNameInput">Nama Cairan</label>
            <input type="text" id="fluidNameInput" class="form-input" placeholder="Misal: Solar / Minyak Goreng" required>
          </div>

          <div class="form-group">
            <label for="unitNameInput">Satuan Pengukuran</label>
            <input type="text" id="unitNameInput" class="form-input" placeholder="Liter, mL, atau kg" required>
          </div>

          <div class="form-group">
            <label for="kFactorInput">K-Factor (Pulsa per Satuan)</label>
            <input type="number" step="0.001" id="kFactorInput" class="form-input" required>
          </div>

          <div class="form-group">
            <label for="priceInput">Harga per Satuan</label>
            <input type="number" step="100" id="priceInput" class="form-input" required>
          </div>

          <div class="form-group full">
            <label for="currencyInput">Simbol Mata Uang</label>
            <input type="text" id="currencyInput" class="form-input" value="Rp" required>
          </div>
        </div>

        <button type="submit" class="btn btn-save">SIMPAN PROFIL KE ESP32</button>
      </form>

      <!-- Asisten Kalibrasi Bejana Ukur -->
      <div class="calib-box">
        <strong style="color:white; display:block; margin-bottom:6px;">&#129514; Asisten Kalibrasi Mandiri (Bejana Ukur / Timbangan):</strong>
        Setiap cairan memiliki viskositas berbeda. Untuk akurasi tinggi, isi bejana ukur / timbang wadah (misal 5.00 Liter):
        <div style="display:flex; gap:8px; margin-top:8px;">
          <input type="number" step="0.01" id="actualVol" class="form-input" placeholder="Isi Asli (Misal 5.00)" style="font-size:0.85rem; padding:8px;">
          <input type="number" step="0.01" id="sensorVol" class="form-input" placeholder="Layar Baca (Misal 4.90)" style="font-size:0.85rem; padding:8px;">
        </div>
        <button type="button" class="btn" style="background:#0284c7; color:white; margin-top:8px; font-size:0.85rem; padding:10px; width:100%;" onclick="calculateAutoCalibration()">
          HITUNG & TERAPKAN K-FACTOR OTOMATIS
        </button>
      </div>

      <!-- Reset Odometer -->
      <div style="margin-top:16px; border-top:1px solid var(--card-border); padding-top:14px;">
        <label style="color:var(--danger); font-size:0.82rem; font-weight:600;">Zona Bahaya:</label>
        <button type="button" class="btn" style="background:rgba(239, 68, 68, 0.15); border:1px solid var(--danger); color:var(--danger); width:100%; margin-top:6px;" onclick="resetTotalLifetime()">
          Nolkan Total Akumulasi Seumur Hidup
        </button>
      </div>
    </div>

    <footer>
      ESP32-S3 N8R16 &bull; AICHI Positive Displacement Flow Sensor &bull; Multi-Fluid Supported
    </footer>
  </div>

  <script>
    let activeCurrency = "Rp";
    let activeUnit = "Liter";

    const FLUID_PRESETS = {
      solar: {
        name: "Solar / Diesel",
        unit: "Liter",
        kFactor: 400.0,
        price: 6800,
        currency: "Rp"
      },
      bensin: {
        name: "Bensin / Gas",
        unit: "Liter",
        kFactor: 400.0,
        price: 10000,
        currency: "Rp"
      },
      minyak_goreng: {
        name: "Minyak Goreng",
        unit: "Liter",
        kFactor: 398.0, // Viskositas sedikit lebih kental
        price: 16000,
        currency: "Rp"
      },
      oli: {
        name: "Oli Pelumas",
        unit: "Liter",
        kFactor: 395.0, // Oli mesin SAE 10W-40 / SAE 40
        price: 45000,
        currency: "Rp"
      },
      susu: {
        name: "Susu Segar",
        unit: "Liter",
        kFactor: 400.0,
        price: 15000,
        currency: "Rp"
      },
      minyak_tanah: {
        name: "Minyak Tanah",
        unit: "Liter",
        kFactor: 400.0,
        price: 12500,
        currency: "Rp"
      },
      air: {
        name: "Air Bersih",
        unit: "Liter",
        kFactor: 400.0,
        price: 1000,
        currency: "Rp"
      },
      custom: {
        name: "Cairan Kustom",
        unit: "Liter",
        kFactor: 400.0,
        price: 10000,
        currency: "Rp"
      }
    };

    function formatCurrency(number) {
      return activeCurrency + ' ' + Math.round(number).toString().replace(/\B(?=(\d{3})+(?!\d))/g, ".");
    }

    function applyPreset(presetKey) {
      const p = FLUID_PRESETS[presetKey];
      if (!p) return;

      document.getElementById('fluidNameInput').value = p.name;
      document.getElementById('unitNameInput').value = p.unit;
      document.getElementById('kFactorInput').value = p.kFactor;
      document.getElementById('priceInput').value = p.price;
      document.getElementById('currencyInput').value = p.currency;

      alert("Preset dipilih: " + p.name + "\nK-Factor rekomendasi: " + p.kFactor + " pulsa/" + p.unit + "\nKlik 'SIMPAN PROFIL KE ESP32' untuk menerapkan.");
    }

    async function fetchRealtimeData() {
      try {
        const response = await fetch('/api/data');
        if (!response.ok) return;
        const data = await response.json();

        activeCurrency = data.currency || "Rp";
        activeUnit = data.unit_name || "Liter";

        // Update Labels & Header
        document.getElementById('topFluidBadge').innerText = data.fluid_name || "Universal";
        document.getElementById('batchUnitLabel').innerText = activeUnit.toUpperCase();
        document.getElementById('flowRateUnitLabel').innerText = activeUnit.toUpperCase() + ' / MENIT';
        document.getElementById('totalUnit').innerText = activeUnit;
        document.getElementById('hourUnit').innerText = activeUnit + '/h';
        document.getElementById('kUnit').innerText = 'pls/' + activeUnit;
        document.getElementById('unitPriceLabel').innerText = '@ ' + formatCurrency(data.price_per_liter) + '/' + activeUnit;

        // Update Gauges
        document.getElementById('batchVolume').innerText = data.batch_liters.toFixed(3);
        document.getElementById('batchCost').innerText = formatCurrency(data.batch_cost);
        document.getElementById('flowRate').innerText = data.flow_rate_lpm.toFixed(2);
        document.getElementById('flowRateHour').innerText = (data.flow_rate_lpm * 60).toFixed(1);
        document.getElementById('totalVolume').innerHTML = data.total_liters.toFixed(2) + ' <span class="metric-unit">' + activeUnit + '</span>';
        document.getElementById('batchPulses').innerHTML = data.batch_pulses + ' <span class="metric-unit">pls</span>';
        document.getElementById('dispKFactor').innerHTML = data.k_factor.toFixed(1) + ' <span class="metric-unit">pls/' + activeUnit + '</span>';

        // Flow status animation
        const flowDot = document.getElementById('flowDot');
        const statusText = document.getElementById('flowStatusText');
        if (data.flow_rate_lpm > 0.05) {
          flowDot.classList.add('active');
          statusText.innerText = 'MENGALIR (' + data.flow_rate_lpm.toFixed(2) + ' ' + activeUnit + '/m)';
          document.getElementById('flowStatusBadge').style.color = '#38bdf8';
          document.getElementById('flowStatusBadge').style.borderColor = 'rgba(56, 189, 248, 0.4)';
        } else {
          flowDot.classList.remove('active');
          statusText.innerText = 'STANDBY';
          document.getElementById('flowStatusBadge').style.color = '#10b981';
          document.getElementById('flowStatusBadge').style.borderColor = 'rgba(16, 185, 129, 0.3)';
        }

        // Fill form fields if empty
        if (!document.getElementById('fluidNameInput').value) {
          document.getElementById('fluidNameInput').value = data.fluid_name || "Solar / Diesel";
        }
        if (!document.getElementById('unitNameInput').value) {
          document.getElementById('unitNameInput').value = data.unit_name || "Liter";
        }
        if (!document.getElementById('priceInput').value) {
          document.getElementById('priceInput').value = data.price_per_liter;
        }
        if (!document.getElementById('kFactorInput').value) {
          document.getElementById('kFactorInput').value = data.k_factor;
        }
        if (!document.getElementById('currencyInput').value) {
          document.getElementById('currencyInput').value = data.currency || "Rp";
        }

      } catch (err) {
        console.warn('Gagal mengambil data:', err);
      }
    }

    async function resetBatch() {
      if (confirm("Nolkan pengukuran saat ini untuk pengisian baru?")) {
        try {
          const res = await fetch('/api/reset_batch', { method: 'POST' });
          if (res.ok) {
            document.getElementById('batchVolume').innerText = "0.000";
            document.getElementById('batchCost').innerText = activeCurrency + " 0";
          }
        } catch(e) {
          alert("Gagal reset batch: " + e);
        }
      }
    }

    async function resetTotalLifetime() {
      const pin = prompt("PERINGATAN: Ini akan mereset total seumur hidup menjadi 0!\nKetik 'RESET' untuk konfirmasi:");
      if (pin === "RESET") {
        try {
          const res = await fetch('/api/reset_total', { method: 'POST' });
          if (res.ok) {
            alert("Total akumulasi berhasil dinolkan.");
          }
        } catch(e) {
          alert("Gagal reset total: " + e);
        }
      }
    }

    function toggleSettings() {
      const card = document.getElementById('settingsCard');
      card.style.display = (card.style.display === 'none') ? 'block' : 'none';
      if (card.style.display === 'block') {
        card.scrollIntoView({ behavior: 'smooth' });
      }
    }

    async function saveSettings(e) {
      e.preventDefault();
      const fluidName = document.getElementById('fluidNameInput').value;
      const unitName = document.getElementById('unitNameInput').value;
      const price = parseFloat(document.getElementById('priceInput').value);
      const kFactor = parseFloat(document.getElementById('kFactorInput').value);
      const currency = document.getElementById('currencyInput').value;

      if (isNaN(price) || isNaN(kFactor) || kFactor <= 0) {
        alert("Input K-Factor atau Harga tidak valid!");
        return;
      }

      try {
        const res = await fetch('/api/settings', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            fluid_name: fluidName,
            unit_name: unitName,
            price: price,
            k_factor: kFactor,
            currency: currency
          })
        });
        if (res.ok) {
          alert("Profil cairan & kalibrasi berhasil disimpan ke ESP32!");
          fetchRealtimeData();
        } else {
          alert("Gagal menyimpan ke ESP32.");
        }
      } catch(err) {
        alert("Koneksi gagal: " + err);
      }
    }

    function calculateAutoCalibration() {
      const actual = parseFloat(document.getElementById('actualVol').value);
      const sensor = parseFloat(document.getElementById('sensorVol').value);
      const currentK = parseFloat(document.getElementById('kFactorInput').value);

      if (!actual || !sensor || !currentK || actual <= 0 || sensor <= 0) {
        alert("Silakan masukkan Nilai Asli Bejana/Timbangan dan Nilai Baca Layar!");
        return;
      }

      // Rumus Kalibrasi: K_baru = K_lama * (sensor / actual)
      const newK = currentK * (sensor / actual);
      document.getElementById('kFactorInput').value = newK.toFixed(3);
      alert("K-Factor baru dihitung: " + newK.toFixed(3) + "\nSilakan klik 'SIMPAN PROFIL KE ESP32' untuk menerapkan.");
    }

    // Refresh data loop every 300 ms for smooth real-time response
    setInterval(fetchRealtimeData, 300);
    fetchRealtimeData();
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
