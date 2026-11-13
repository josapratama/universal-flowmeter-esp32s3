#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
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
    *, *::before, *::after {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    }
    html, body {
      overflow-x: hidden;
      width: 100%;
      max-width: 100vw;
      background-color: var(--bg);
      color: var(--text);
    }
    body {
      padding: 12px;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .container {
      width: 100%;
      max-width: 860px;
      overflow-x: hidden;
      display: flex;
      flex-direction: column;
      gap: 14px;
      margin: 0 auto;
    }
    
    /* Header */
    header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px 16px;
      background: var(--card-bg);
      border-radius: 16px;
      border: 1px solid var(--card-border);
      flex-wrap: wrap;
      gap: 10px;
      width: 100%;
    }
    .brand { display: flex; align-items: center; gap: 12px; }
    .brand-icon {
      width: 42px;
      height: 42px;
      background: linear-gradient(135deg, #0ea5e9, #6366f1);
      border-radius: 12px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 20px;
      box-shadow: 0 4px 14px rgba(14, 165, 233, 0.4);
      flex-shrink: 0;
    }
    .brand-text h1 { font-size: 1.1rem; font-weight: 700; line-height: 1.2; }
    .fluid-badge {
      display: inline-block;
      font-size: 0.72rem;
      font-weight: 700;
      padding: 2px 8px;
      border-radius: 6px;
      background: rgba(245, 158, 11, 0.15);
      color: #f59e0b;
      border: 1px solid rgba(245, 158, 11, 0.3);
      margin-top: 2px;
    }
    .header-badges {
      display: flex;
      align-items: center;
      gap: 6px;
      flex-wrap: wrap;
    }
    .badge {
      padding: 4px 10px;
      border-radius: 9999px;
      font-size: 0.7rem;
      font-weight: 700;
      display: flex;
      align-items: center;
      gap: 6px;
      white-space: nowrap;
    }
    .badge-valve-closed { background: rgba(239, 68, 68, 0.15); color: var(--danger); border: 1px solid rgba(239, 68, 68, 0.3); }
    .badge-valve-open { background: rgba(16, 185, 129, 0.2); color: var(--success); border: 1px solid rgba(16, 185, 129, 0.4); animation: pulse-border 1.2s infinite; }
    .badge-alarm { background: rgba(239, 68, 68, 0.25); color: #ff5252; border: 1px solid #ef4444; animation: flash-red 0.8s infinite; cursor: pointer; }
    @keyframes pulse-border { 0%, 100% { box-shadow: 0 0 0 0 rgba(16, 185, 129, 0.4); } 50% { box-shadow: 0 0 0 6px rgba(16, 185, 129, 0); } }
    @keyframes flash-red { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }

    /* Fluid Presets Quick Bar (Bank Cairan Siap Pakai) */
    .fluid-bar {
      display: flex;
      gap: 6px;
      overflow-x: auto;
      padding: 4px 2px;
      width: 100%;
      -webkit-overflow-scrolling: touch;
      scrollbar-width: none;
    }
    .fluid-bar::-webkit-scrollbar { display: none; }
    .fluid-btn {
      flex: 0 0 auto;
      background: #0f172a;
      border: 1px solid #334155;
      color: #cbd5e1;
      padding: 7px 12px;
      border-radius: 10px;
      font-size: 0.78rem;
      font-weight: 700;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 6px;
      transition: all 0.2s;
      white-space: nowrap;
    }
    .fluid-btn:hover { border-color: var(--primary); color: #fff; }
    .fluid-btn.active {
      background: rgba(14, 165, 233, 0.2);
      border-color: var(--primary);
      color: #38bdf8;
      box-shadow: 0 0 10px rgba(14, 165, 233, 0.25);
    }

    /* Main Big Display (SPBU Style) */
    .dispenser-display {
      background: var(--disp-bg);
      border: 2px solid #334155;
      border-radius: 20px;
      padding: 18px;
      box-shadow: inset 0 2px 10px rgba(0,0,0,0.8), 0 10px 25px -5px rgba(0,0,0,0.5);
      display: flex;
      flex-direction: column;
      gap: 14px;
      width: 100%;
      box-sizing: border-box;
      overflow: hidden;
    }
    .disp-row {
      display: flex;
      justify-content: space-between;
      align-items: flex-end;
      padding-bottom: 12px;
      border-bottom: 1px dashed #1e293b;
      gap: 10px;
    }
    .disp-row:last-child { border-bottom: none; padding-bottom: 0; }
    .disp-label { font-size: 0.8rem; font-weight: 800; letter-spacing: 0.5px; color: var(--text-muted); text-transform: uppercase; }
    .disp-value {
      font-family: "Courier New", Courier, monospace;
      font-size: clamp(1.8rem, 7vw, 2.8rem);
      font-weight: 900;
      line-height: 1;
      color: #38bdf8;
      text-shadow: 0 0 12px rgba(56, 189, 248, 0.5);
      word-break: break-all;
    }
    .disp-cost {
      font-family: "Courier New", Courier, monospace;
      font-size: clamp(1.6rem, 6vw, 2.6rem);
      font-weight: 900;
      line-height: 1;
      color: #f59e0b;
      text-shadow: 0 0 12px rgba(245, 158, 11, 0.5);
      word-break: break-all;
    }
    .disp-unit { font-size: 1rem; color: #64748b; font-weight: 700; margin-left: 4px; }

    /* Flow Wave Animation Bar */
    .flow-wave-bar {
      height: 6px;
      width: 100%;
      background: #1e293b;
      border-radius: 999px;
      overflow: hidden;
      position: relative;
    }
    .flow-wave-fill {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, #0ea5e9, #38bdf8, #10b981);
      border-radius: 999px;
      transition: width 0.3s;
    }
    .flow-wave-bar.active .flow-wave-fill {
      width: 100%;
      background: linear-gradient(90deg, #0ea5e9, #67e8f9, #10b981, #0ea5e9);
      background-size: 200% 100%;
      animation: wave-run 1s linear infinite;
    }
    @keyframes wave-run { 0% { background-position: 100% 0; } 100% { background-position: -100% 0; } }

    /* Gauge & Mini Stats Grid */
    .metrics-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(130px, 1fr));
      gap: 10px;
      width: 100%;
    }
    .stat-box {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 12px;
      padding: 12px;
      text-align: center;
      min-width: 0;
    }
    .stat-label { font-size: 0.68rem; color: var(--text-muted); text-transform: uppercase; font-weight: 700; }
    .stat-val { font-size: 1.15rem; font-weight: 800; color: #f1f5f9; margin-top: 2px; }

    /* Shift & Today Summary Cards */
    .summary-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      width: 100%;
    }
    @media (max-width: 580px) {
      .summary-grid { grid-template-columns: 1fr; }
    }
    .summary-card {
      background: #0b1120;
      border: 1px solid #1e293b;
      border-radius: 14px;
      padding: 12px 14px;
      display: flex;
      flex-direction: column;
      gap: 6px;
    }
    .summary-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 0.78rem;
      font-weight: 700;
      color: #94a3b8;
    }
    .summary-data {
      display: flex;
      justify-content: space-between;
      align-items: baseline;
      gap: 8px;
    }
    .summary-vol { font-size: 1.25rem; font-weight: 900; color: #38bdf8; font-family: monospace; }
    .summary-cost { font-size: 1.05rem; font-weight: 800; color: #f59e0b; }

    /* Auto Cut-off Control Station */
    .control-card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 16px;
      padding: 16px;
      display: flex;
      flex-direction: column;
      gap: 14px;
      width: 100%;
      box-sizing: border-box;
      overflow: hidden;
    }
    .control-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 8px;
    }
    .control-title {
      font-size: 0.92rem;
      font-weight: 700;
      color: #e2e8f0;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .mode-tabs {
      display: flex;
      background: #0b1120;
      padding: 4px;
      border-radius: 10px;
      gap: 4px;
      border: 1px solid #1e293b;
      flex-wrap: wrap;
    }
    .mode-tab {
      flex: 1;
      text-align: center;
      font-size: 0.76rem;
      font-weight: 700;
      padding: 6px 10px;
      border-radius: 8px;
      cursor: pointer;
      color: var(--text-muted);
      transition: all 0.2s;
      border: none;
      background: transparent;
      white-space: nowrap;
    }
    .mode-tab.active { background: var(--primary); color: #fff; }

    /* Preset Chips */
    .preset-chips { display: flex; flex-wrap: wrap; gap: 8px; width: 100%; }
    .chip {
      padding: 8px 14px;
      border-radius: 10px;
      font-size: 0.82rem;
      font-weight: 700;
      background: #0b1120;
      border: 1px solid #334155;
      color: #e2e8f0;
      cursor: pointer;
      transition: all 0.2s;
    }
    .chip:hover { border-color: var(--primary); color: var(--primary); }
    .chip.active { background: rgba(14, 165, 233, 0.2); border-color: var(--primary); color: #38bdf8; }

    /* Target Input & Action Buttons */
    .target-bar {
      display: flex;
      gap: 10px;
      align-items: center;
      width: 100%;
      box-sizing: border-box;
    }
    @media (max-width: 480px) {
      .target-bar { flex-direction: column; align-items: stretch; }
    }
    .target-input {
      flex: 1;
      min-width: 0;
      width: 100%;
      background: #0b1120;
      border: 1px solid #334155;
      border-radius: 10px;
      padding: 10px 14px;
      font-size: 1.1rem;
      font-weight: 700;
      color: #fff;
      outline: none;
      box-sizing: border-box;
    }
    .target-input:focus { border-color: var(--primary); }

    .btn-action-group {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      width: 100%;
      box-sizing: border-box;
    }
    @media (max-width: 480px) {
      .btn-action-group { grid-template-columns: 1fr; }
    }
    .btn-start {
      background: linear-gradient(135deg, #10b981, #059669);
      color: #fff;
      border: none;
      border-radius: 12px;
      padding: 13px;
      font-size: 0.95rem;
      font-weight: 800;
      cursor: pointer;
      transition: all 0.2s;
      box-shadow: 0 4px 12px rgba(16, 185, 129, 0.3);
      text-align: center;
    }
    .btn-start:hover { opacity: 0.9; transform: translateY(-1px); }
    .btn-start:disabled { opacity: 0.5; cursor: not-allowed; }
    .btn-stop {
      background: linear-gradient(135deg, #ef4444, #dc2626);
      color: #fff;
      border: none;
      border-radius: 12px;
      padding: 13px;
      font-size: 0.95rem;
      font-weight: 800;
      cursor: pointer;
      transition: all 0.2s;
      box-shadow: 0 4px 12px rgba(239, 68, 68, 0.3);
      text-align: center;
    }
    .btn-stop:hover { opacity: 0.9; transform: translateY(-1px); }
    .btn-secondary {
      background: #1e293b;
      color: #cbd5e1;
      border: 1px solid #334155;
      border-radius: 10px;
      padding: 8px 12px;
      font-size: 0.8rem;
      font-weight: 700;
      cursor: pointer;
      transition: all 0.2s;
      white-space: nowrap;
    }
    .btn-secondary:hover { background: #334155; color: #fff; }

    /* Responsive Form Grid (Fix Overflow Bug) */
    .form-grid-2 {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
      width: 100%;
      box-sizing: border-box;
    }
    @media (max-width: 600px) {
      .form-grid-2 {
        grid-template-columns: 1fr;
        gap: 8px;
      }
    }
    .form-group {
      display: flex;
      flex-direction: column;
      gap: 5px;
      margin-bottom: 10px;
      width: 100%;
      min-width: 0;
      box-sizing: border-box;
    }
    .form-group label {
      font-size: 0.74rem;
      font-weight: 700;
      color: var(--text-muted);
      text-transform: uppercase;
      word-break: break-word;
    }
    .form-input {
      background: #0b1120;
      border: 1px solid #334155;
      border-radius: 8px;
      padding: 8px 12px;
      font-size: 0.85rem;
      color: #fff;
      outline: none;
      width: 100%;
      min-width: 0;
      box-sizing: border-box;
    }
    .form-input:focus { border-color: var(--primary); }

    /* Tabs Header */
    .tabs-header {
      display: flex;
      gap: 6px;
      border-bottom: 1px solid var(--card-border);
      padding-bottom: 8px;
      overflow-x: auto;
      width: 100%;
      -webkit-overflow-scrolling: touch;
      scrollbar-width: thin;
    }
    .tab-btn {
      background: transparent;
      border: none;
      font-size: 0.82rem;
      font-weight: 700;
      color: var(--text-muted);
      cursor: pointer;
      padding: 6px 12px;
      border-radius: 8px;
      white-space: nowrap;
      flex-shrink: 0;
    }
    .tab-btn.active { color: var(--primary); background: rgba(14, 165, 233, 0.12); }
    .tab-content { display: none; width: 100%; box-sizing: border-box; }
    .tab-content.active { display: block; }

    /* Table */
    .table-responsive {
      width: 100%;
      overflow-x: auto;
      -webkit-overflow-scrolling: touch;
    }
    table { width: 100%; border-collapse: collapse; font-size: 0.76rem; text-align: left; margin-top: 10px; min-width: 520px; }
    th { padding: 8px 10px; background: #0b1120; color: var(--text-muted); font-weight: 700; border-bottom: 1px solid var(--card-border); }
    td { padding: 8px 10px; border-bottom: 1px solid #1e293b; }

    /* Modal Struk SPBU */
    .modal-overlay {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      width: 100vw;
      height: 100vh;
      background: rgba(0,0,0,0.75);
      z-index: 999;
      justify-content: center;
      align-items: center;
      padding: 14px;
      box-sizing: border-box;
    }
    .receipt-box {
      background: #fff;
      color: #111;
      border-radius: 10px;
      padding: 20px;
      width: 100%;
      max-width: 360px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.5);
      font-family: monospace;
      font-size: 13px;
      line-height: 1.4;
      box-sizing: border-box;
    }
    .receipt-header { text-align: center; border-bottom: 1px dashed #444; padding-bottom: 8px; margin-bottom: 8px; }
    .receipt-header h3 { font-size: 16px; margin-bottom: 2px; }
    .receipt-row { display: flex; justify-content: space-between; margin-bottom: 4px; }
    .receipt-divider { border-top: 1px dashed #444; margin: 8px 0; }
    .receipt-actions { display: flex; gap: 8px; margin-top: 14px; }

    /* Progress Bar */
    .progress-bar-bg { width: 100%; height: 12px; background: #0b1120; border-radius: 6px; overflow: hidden; border: 1px solid #334155; margin: 10px 0; }
    .progress-bar-fill { height: 100%; width: 0%; background: linear-gradient(90deg, #0ea5e9, #10b981); transition: width 0.2s; }

    @media print {
      body * { visibility: hidden; }
      .receipt-box, .receipt-box * { visibility: visible; }
      .receipt-box { position: absolute; left: 0; top: 0; width: 100%; max-width: 320px; box-shadow: none; padding: 5px; }
      .receipt-actions { display: none !important; }
    }
    
    footer { text-align: center; font-size: 0.72rem; color: #64748b; padding: 14px 0; }
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
          <span class="fluid-badge" id="topFluidBadge">Bio Solar Subsidi</span>
        </div>
      </div>
      <div class="header-badges">
        <div class="badge badge-valve-closed" id="valveStatusBadge">
          <span style="font-size:10px;">●</span> <span id="valveStatusText">VALVE TERTUTUP</span>
        </div>
        <div class="badge badge-alarm" id="alarmBadge" style="display:none;" onclick="clearAlerts()">
          ⚠️ <span id="alarmText">ALARM</span> [Reset]
        </div>
        <div class="badge" style="background:#0b1120;border:1px solid #334155;color:#94a3b8;" id="clockBadge">
          🕒 <span id="clockText">Memuat jam...</span>
        </div>
        <div class="badge" style="background:#0b1120;border:1px solid #334155;color:#94a3b8;" id="wifiBadge">
          📶 <span id="wifiStatusText">AP: 192.168.4.1</span>
        </div>
      </div>
    </header>

    <!-- Quick Fluid Selector Bar (Bank Cairan Siap Pakai) -->
    <div class="fluid-bar" id="fluidQuickBar">
      <button class="fluid-btn active" onclick="quickSelectFluid('biosolar')">⛽ Bio Solar (Rp6.800)</button>
      <button class="fluid-btn" onclick="quickSelectFluid('pertalite')">⛽ Pertalite (Rp10.000)</button>
      <button class="fluid-btn" onclick="quickSelectFluid('pertamax')">⛽ Pertamax (Rp12.100)</button>
      <button class="fluid-btn" onclick="quickSelectFluid('dexlite')">⛽ Dexlite (Rp13.050)</button>
      <button class="fluid-btn" onclick="quickSelectFluid('minyak_goreng')">🛢️ Minyak Curah (Rp15.500)</button>
      <button class="fluid-btn" onclick="quickSelectFluid('air_bersih')">💧 Air Bersih (Rp50)</button>
    </div>

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

      <!-- Realtime Flow Wave Animation -->
      <div class="flow-wave-bar" id="flowWaveBar">
        <div class="flow-wave-fill"></div>
      </div>

      <!-- Quick Action Under Dispenser: Instant Receipt Button -->
      <div style="display:flex;justify-content:space-between;align-items:center;padding-top:4px;">
        <span style="font-size:0.75rem;color:#64748b;" id="liveKFactorLabel">K: 400.0 pulsa/L</span>
        <button class="btn-secondary" style="background:#131b2e;border-color:#38bdf8;color:#38bdf8;" onclick="printLastBatchReceipt()">
          🖨️ Cetak Struk Terakhir
        </button>
      </div>
    </div>

    <!-- Mini Metrics Grid -->
    <div class="metrics-grid">
      <div class="stat-box" style="display:flex;flex-direction:column;align-items:center;justify-content:center;padding:8px 10px;">
        <div class="stat-label">SPEEDOMETER DEBIT</div>
        <svg viewBox="0 0 100 58" style="width:100%;max-width:120px;display:block;margin-top:2px;">
          <defs>
            <linearGradient id="gaugeGrad" x1="0%" y1="0%" x2="100%" y2="0%">
              <stop offset="0%" stop-color="#0ea5e9" />
              <stop offset="70%" stop-color="#10b981" />
              <stop offset="100%" stop-color="#ef4444" />
            </linearGradient>
          </defs>
          <path d="M 12 50 A 38 38 0 0 1 88 50" fill="none" stroke="#1e293b" stroke-width="8" stroke-linecap="round" />
          <path id="gaugePath" d="M 12 50 A 38 38 0 0 1 88 50" fill="none" stroke="url(#gaugeGrad)" stroke-width="8" stroke-linecap="round" stroke-dasharray="119.4" stroke-dashoffset="119.4" style="transition: stroke-dashoffset 0.25s;" />
          <text id="gaugeText" x="50" y="46" text-anchor="middle" font-size="12" font-weight="900" fill="#38bdf8">0.0</text>
          <text x="50" y="55" text-anchor="middle" font-size="6" font-weight="700" fill="#64748b">LPM</text>
        </svg>
      </div>
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
        <div class="stat-label">KOREKSI CUT-OFF</div>
        <div class="stat-val" style="color:#f59e0b;" id="overshootDisplay">20 <span style="font-size:0.7rem;color:#64748b;">mL</span></div>
      </div>
    </div>

    <!-- Shift & Today Summary Cards -->
    <div class="summary-grid">
      <div class="summary-card">
        <div class="summary-header">
          <span>👔 SHIFT OPERATOR SAAT INI</span>
          <button class="btn-secondary" style="padding:2px 8px;font-size:0.7rem;color:#ef4444;border-color:rgba(239,68,68,0.3);" onclick="resetShift()">
            🔄 Reset Shift
          </button>
        </div>
        <div class="summary-data">
          <div>
            <span class="summary-vol" id="shiftVol">0.00</span> <span style="font-size:0.75rem;color:#64748b;">L</span>
          </div>
          <div class="summary-cost" id="shiftCost">Rp 0</div>
        </div>
        <div style="font-size:0.7rem;color:#64748b;" id="shiftTxCount">0 transaksi pengisian</div>
      </div>

      <div class="summary-card">
        <div class="summary-header">
          <span>📅 TOTAL HARI INI (<span id="todayDateStr">Hari Ini</span>)</span>
          <span style="font-size:0.7rem;color:#10b981;">● Auto-Reset</span>
        </div>
        <div class="summary-data">
          <div>
            <span class="summary-vol" id="todayVol">0.00</span> <span style="font-size:0.75rem;color:#64748b;">L</span>
          </div>
          <div class="summary-cost" id="todayCost">Rp 0</div>
        </div>
        <div style="font-size:0.7rem;color:#64748b;" id="todayTxCount">0 transaksi pengisian</div>
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

      <!-- Target input (Responsive & Anti-Overflow) -->
      <div class="target-bar" id="targetInputGroup">
        <input type="number" step="0.1" class="target-input" id="targetInput" placeholder="Target Volume..." value="5.0" />
        <span style="font-weight:700;color:var(--text-muted);white-space:nowrap;" id="targetUnitDisplay">Liter</span>
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

      <div style="display:flex;gap:8px;justify-content:flex-end;margin-top:2px;">
        <button class="btn-secondary" onclick="resetBatch()">
          🔄 Nolkan Pengisian (Reset Batch)
        </button>
      </div>
    </div>

    <!-- Multi Tab Section: Riwayat | Profil | Kalibrasi 3-Titik | Bejana | WiFi | Web OTA -->
    <div class="control-card">
      <div class="tabs-header">
        <button class="tab-btn active" onclick="switchTab('historyTab', this)">📜 Riwayat Transaksi</button>
        <button class="tab-btn" onclick="switchTab('profileTab', this)">⚙️ Profil Cairan</button>
        <button class="tab-btn" onclick="switchTab('multiPointTab', this)">📈 Kalibrasi 3-Titik</button>
        <button class="tab-btn" onclick="switchTab('bejanaTab', this)">🧪 Wizard Bejana</button>
        <button class="tab-btn" onclick="switchTab('wifiTab', this)">📶 WiFi & IoT</button>
        <button class="tab-btn" onclick="switchTab('otaTab', this)">🚀 Update OTA</button>
      </div>

      <!-- TAB 1: Riwayat Transaksi -->
      <div class="tab-content active" id="historyTab">
        <div style="display:flex;justify-content:space-between;align-items:center;margin:10px 0;flex-wrap:wrap;gap:8px;">
          <span style="font-size:0.8rem;color:var(--text-muted);">Riwayat 50 pengisian terakhir otomatis tersimpan di Flash NVS</span>
          <div style="display:flex;gap:8px;flex-wrap:wrap;">
            <a href="/api/history/export" class="btn-secondary" style="text-decoration:none;display:inline-flex;align-items:center;gap:6px;">
              📥 Download CSV
            </a>
            <button class="btn-secondary" onclick="clearHistory()" style="color:#ef4444;">
              🗑️ Hapus
            </button>
          </div>
        </div>
        <div class="table-responsive">
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

      <!-- TAB 2: Profil Cairan & Kalibrasi Dasar (Responsive form-grid-2) -->
      <div class="tab-content" id="profileTab">
        <div style="padding:10px 0;">
          <div class="form-group">
            <label>Katalog Preset Cairan Cepat:</label>
            <select class="form-input" onchange="quickSelectFluid(this.value)">
              <option value="">-- Pilih Preset Bahan Bakar / Cairan --</option>
              <option value="biosolar">⛽ Bio Solar Subsidi (Rp 6.800 / L)</option>
              <option value="pertalite">⛽ Pertalite RON 90 (Rp 10.000 / L)</option>
              <option value="pertamax">⛽ Pertamax RON 92 (Rp 12.100 / L)</option>
              <option value="dexlite">⛽ Dexlite (Rp 13.050 / L)</option>
              <option value="minyak_goreng">🛢️ Minyak Goreng Curah (Rp 15.500 / L)</option>
              <option value="air_bersih">💧 Air Bersih / RO (Rp 50 / L)</option>
            </select>
          </div>

          <div class="form-grid-2">
            <div class="form-group">
              <label>Nama Cairan / Fluida:</label>
              <input type="text" class="form-input" id="cfgFluidName" value="Solar / Diesel" />
            </div>
            <div class="form-group">
              <label>Satuan Volume:</label>
              <input type="text" class="form-input" id="cfgUnitName" value="Liter" />
            </div>
          </div>

          <div class="form-grid-2">
            <div class="form-group">
              <label>Mata Uang:</label>
              <input type="text" class="form-input" id="cfgCurrency" value="Rp" />
            </div>
            <div class="form-group">
              <label>Harga per Satuan (Rp):</label>
              <input type="number" step="100" class="form-input" id="cfgPrice" value="6800" />
            </div>
          </div>

          <div class="form-grid-2">
            <div class="form-group">
              <label>K-Factor Sensor (pulsa/L):</label>
              <input type="number" step="0.1" class="form-input" id="cfgKFactor" value="400.0" />
            </div>
            <div class="form-group">
              <label>Kompensasi Lead Cut-off (Liter):</label>
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

          <div style="display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:8px;">
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

      <!-- TAB 3: Kalibrasi Multi-Titik (3-Point Flow Curve) -->
      <div class="tab-content" id="multiPointTab">
        <div style="padding:10px 0;">
          <h4 style="font-size:0.95rem;color:#38bdf8;margin-bottom:6px;">📈 Kalibrasi Non-Linear 3-Titik (Presisi <0.2%)</h4>
          <p style="font-size:0.8rem;color:var(--text-muted);line-height:1.4;margin-bottom:14px;">
            Sensor flowmeter AICHI memiliki karakteristik sedikit berbeda antara debit rendah (gravitasi / kran kecil) dan debit tinggi (pompa dorong penuh). Sesuaikan K-Factor untuk tiap rentang debit.
          </p>

          <div class="form-group">
            <label style="display:flex;align-items:center;gap:8px;cursor:pointer;">
              <input type="checkbox" id="cfgMpEnabled" checked style="width:18px;height:18px;" />
              <span>Aktifkan Kalibrasi Kurva Dinamis 3-Titik</span>
            </label>
          </div>

          <div class="form-grid-2">
            <div class="form-group">
              <label>K-Factor Debit Rendah (&lt; 5 LPM):</label>
              <input type="number" step="0.1" class="form-input" id="cfgKLow" value="396.0" />
            </div>
            <div class="form-group">
              <label>K-Factor Debit Normal (5 - 15 LPM):</label>
              <input type="number" step="0.1" class="form-input" id="cfgKMid" value="400.0" />
            </div>
          </div>

          <div class="form-grid-2">
            <div class="form-group">
              <label>K-Factor Debit Tinggi (&gt; 15 LPM):</label>
              <input type="number" step="0.1" class="form-input" id="cfgKHigh" value="404.0" />
            </div>
            <div class="form-group">
              <label>PIN Admin:</label>
              <input type="password" class="form-input" id="cfgMpPin" placeholder="PIN Admin (Default: 1234)" />
            </div>
          </div>

          <button class="btn-start" style="width:100%;padding:12px;" onclick="saveMultiPointCalibration()">
            🎯 Simpan Kalibrasi 3-Titik
          </button>
        </div>
      </div>

      <!-- TAB 4: Wizard Bejana Ukur -->
      <div class="tab-content" id="bejanaTab">
        <div style="padding:10px 0;">
          <h4 style="font-size:0.95rem;color:#38bdf8;margin-bottom:6px;">🧪 Wizard Kalibrasi Bejana Ukur Presisi</h4>
          <p style="font-size:0.8rem;color:var(--text-muted);line-height:1.4;margin-bottom:14px;">
            Gunakan bejana ukur standar tera (misal 10 Liter atau 20 Liter). Sistem akan menghitung jumlah pulsa sensor aktual dan secara otomatis mengkalkulasi K-Factor baru yang presisi tanpa hitung manual.
          </p>

          <div style="background:#0b1120;padding:12px;border-radius:12px;border:1px solid #1e293b;margin-bottom:14px;">
            <div style="font-size:0.8rem;font-weight:700;color:#e2e8f0;margin-bottom:6px;">Langkah-langkah Kalibrasi:</div>
            <ol style="font-size:0.76rem;color:var(--text-muted);padding-left:18px;line-height:1.5;">
              <li>Siapkan bejana ukur kosong (misal 10 Liter).</li>
              <li>Klik tombol <b>[Nolkan Pengisian (Reset Batch)]</b> agar pulsa kembali ke 0.</li>
              <li>Alirkan cairan menggunakan mode <b>Manual</b> hingga permukaan cairan tepat di garis tera bejana.</li>
              <li>Hentikan aliran, lalu pilih ukuran bejana di bawah dan klik <b>[Terapkan Kalibrasi]</b>.</li>
            </ol>
          </div>

          <div class="form-grid-2">
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
          </div>

          <button class="btn-start" style="width:100%;padding:12px;background:linear-gradient(135deg, #0ea5e9, #2563eb);" onclick="runBejanaCalibration()">
            🎯 Terapkan Kalibrasi Bejana (Hitung K-Factor Baru)
          </button>
        </div>
      </div>

      <!-- TAB 5: WiFi, Jam & IoT Hub -->
      <div class="tab-content" id="wifiTab">
        <div style="padding:10px 0;">
          <h4 style="font-size:0.95rem;color:#38bdf8;margin-bottom:6px;">📶 Koneksi WiFi Stasiun (STA Mode)</h4>
          <p style="font-size:0.8rem;color:var(--text-muted);margin-bottom:14px;">
            Sambungkan ESP32 ke WiFi rumah/kantor untuk sinkronisasi jam NTP otomatis dan integrasi ke server Omni Agent Hub.
          </p>

          <div class="form-grid-2">
            <div class="form-group">
              <label>SSID WiFi Rumah / Kantor:</label>
              <input type="text" class="form-input" id="staSsid" placeholder="Nama WiFi (SSID)" />
            </div>
            <div class="form-group">
              <label>Password WiFi:</label>
              <input type="password" class="form-input" id="staPass" placeholder="Password WiFi" />
            </div>
          </div>

          <h4 style="font-size:0.95rem;color:#f59e0b;margin-top:16px;margin-bottom:6px;">🌐 Webhook Push Omni Agent Hub (IoT Integration)</h4>
          <div class="form-group">
            <label>URL Webhook Server (Dipanggil otomatis tiap selesai pengisian):</label>
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

      <!-- TAB 6: Web OTA Firmware Update -->
      <div class="tab-content" id="otaTab">
        <div style="padding:10px 0;">
          <h4 style="font-size:0.95rem;color:#10b981;margin-bottom:6px;">🚀 Update Firmware via Browser (Web OTA)</h4>
          <p style="font-size:0.8rem;color:var(--text-muted);line-height:1.4;margin-bottom:14px;">
            Unggah file <code>firmware.bin</code> hasil kompilasi langsung melalui browser tanpa perlu mencolok kabel USB ke ESP32.
          </p>

          <div class="form-group">
            <label>Pilih File Firmware (.bin):</label>
            <input type="file" id="otaFileInput" accept=".bin" class="form-input" style="padding:6px;" />
          </div>

          <div class="progress-bar-bg" id="otaProgressBg" style="display:none;">
            <div class="progress-bar-fill" id="otaProgressFill"></div>
          </div>
          <div id="otaStatusText" style="font-size:0.8rem;color:#94a3b8;margin-bottom:10px;"></div>

          <button class="btn-start" id="btnOtaUpload" style="width:100%;padding:12px;background:linear-gradient(135deg, #10b981, #059669);" onclick="startOtaUpload()">
            ⚡ Flash & Upgrade Firmware
          </button>
        </div>
      </div>
    </div>

    <!-- Modal Cetak Struk SPBU (Thermal 58mm / 80mm) -->
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
          <span id="rcptTime">2026-11-13 11:00:00</span>
        </div>
        <div class="receipt-row">
          <span>Plat Nomor</span>
          <input type="text" id="rcptPlate" value="B 1234 ABC" style="font-family:monospace;font-size:12px;border:1px solid #ccc;padding:2px 4px;width:100px;text-align:right;" />
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
          <button class="btn-start" style="flex:1;padding:8px;" onclick="window.print()">🖨️ Cetak</button>
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
    let settingsFormInitialized = false;
    let latestData = {};

    const PRESET_VOLUMES = [1, 2, 5, 10, 20, 50];
    const PRESET_AMOUNTS = [20000, 50000, 100000, 200000, 500000];

    const FLUID_PRESETS = {
      biosolar:       { id: "biosolar", name: "Bio Solar Subsidi", price: 6800, k: 400.0, overshoot: 0.020 },
      pertalite:      { id: "pertalite", name: "Pertalite RON 90", price: 10000, k: 400.0, overshoot: 0.020 },
      pertamax:       { id: "pertamax", name: "Pertamax RON 92", price: 12100, k: 400.0, overshoot: 0.020 },
      dexlite:        { id: "dexlite", name: "Dexlite", price: 13050, k: 400.0, overshoot: 0.020 },
      minyak_goreng:  { id: "minyak_goreng", name: "Minyak Goreng Curah", price: 15500, k: 396.0, overshoot: 0.035 },
      air_bersih:     { id: "air_bersih", name: "Air Bersih / RO", price: 50, k: 400.0, overshoot: 0.010 }
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

    async function quickSelectFluid(presetKey) {
      if (!presetKey) return;
      try {
        const res = await fetch("/api/fluid/preset", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ preset_id: presetKey })
        });
        const d = await res.json();
        if (d.status === "ok") {
          document.querySelectorAll(".fluid-btn").forEach(b => {
            b.classList.toggle("active", b.getAttribute("onclick")?.includes(presetKey));
          });
          fetchData();
        }
      } catch (err) {
        console.error("Gagal ganti profil cairan:", err);
      }
    }

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
        latestData = d;

        autoSyncBrowserTime();

        document.getElementById("topFluidBadge").textContent = d.fluid_name;
        document.getElementById("batchVolume").textContent = Number(d.batch_liters).toFixed(3);
        document.getElementById("batchUnitLabel").textContent = d.unit_name === "Liter" ? "L" : d.unit_name;
        document.getElementById("batchCost").textContent = `${d.currency} ${Math.round(d.batch_cost).toLocaleString('id-ID')}`;
        document.getElementById("unitPriceLabel").textContent = `@ ${d.currency} ${Math.round(d.price_per_liter).toLocaleString('id-ID')} / ${d.unit_name}`;

        // Flow Rates & Wave
        const flowVal = Number(d.flow_rate_lpm);
        document.getElementById("flowRate").innerHTML = `${flowVal.toFixed(2)} <span style="font-size:0.7rem;color:#64748b;">${d.unit_name}/m</span>`;
        document.getElementById("flowRateHour").innerHTML = `${(flowVal * 60).toFixed(1)} <span style="font-size:0.7rem;color:#64748b;">${d.unit_name}/h</span>`;
        document.getElementById("totalVolume").innerHTML = `${Number(d.total_liters).toFixed(2)} <span style="font-size:0.7rem;color:#64748b;">${d.unit_name}</span>`;
        document.getElementById("overshootDisplay").innerHTML = `${Math.round((d.overshoot_liters || 0.02) * 1000)} <span style="font-size:0.7rem;color:#64748b;">mL</span>`;
        document.getElementById("liveKFactorLabel").textContent = `K: ${Number(d.effective_k || d.k_factor).toFixed(1)} pulsa/L`;

        // Speedometer Gauge Update
        const maxLpm = 30.0;
        const lpmClamped = Math.min(Math.max(flowVal, 0), maxLpm);
        const arcLength = 119.4;
        const offset = arcLength - (lpmClamped / maxLpm) * arcLength;
        const gaugeEl = document.getElementById("gaugePath");
        if (gaugeEl) gaugeEl.style.strokeDashoffset = offset;
        const gaugeTxt = document.getElementById("gaugeText");
        if (gaugeTxt) gaugeTxt.textContent = flowVal.toFixed(1);

        // Highlight Active Fluid Preset Button
        if (d.active_fluid_id) {
          document.querySelectorAll(".fluid-btn").forEach(b => {
            b.classList.toggle("active", b.getAttribute("onclick")?.includes(d.active_fluid_id));
          });
        }

        // Auto Populate Settings Form
        if (!settingsFormInitialized) {
          settingsFormInitialized = true;
          if (document.getElementById("cfgFluidName")) document.getElementById("cfgFluidName").value = d.fluid_name || "";
          if (document.getElementById("cfgPrice")) document.getElementById("cfgPrice").value = d.price_per_liter || 0;
          if (document.getElementById("cfgKFactor")) document.getElementById("cfgKFactor").value = Number(d.k_factor || 400).toFixed(1);
          if (document.getElementById("cfgOvershoot")) document.getElementById("cfgOvershoot").value = (d.overshoot_liters !== undefined) ? d.overshoot_liters : 0.02;
          if (document.getElementById("cfgUnitName")) document.getElementById("cfgUnitName").value = d.unit_name || "Liter";
          if (document.getElementById("cfgCurrency")) document.getElementById("cfgCurrency").value = d.currency || "Rp";
          if (document.getElementById("cfgKLow") && d.k_low) document.getElementById("cfgKLow").value = Number(d.k_low).toFixed(1);
          if (document.getElementById("cfgKMid") && d.k_mid) document.getElementById("cfgKMid").value = Number(d.k_mid).toFixed(1);
          if (document.getElementById("cfgKHigh") && d.k_high) document.getElementById("cfgKHigh").value = Number(d.k_high).toFixed(1);
          if (document.getElementById("cfgMpEnabled") && d.mp_enabled !== undefined) document.getElementById("cfgMpEnabled").checked = d.mp_enabled;
        }

        // Wave animation toggle
        const waveEl = document.getElementById("flowWaveBar");
        if (d.is_dispensing || flowVal > 0.1) {
          waveEl.classList.add("active");
        } else {
          waveEl.classList.remove("active");
        }

        // Shift Summary
        document.getElementById("shiftVol").textContent = Number(d.shift_liters || 0).toFixed(2);
        document.getElementById("shiftCost").textContent = `${d.currency} ${Math.round(d.shift_cost || 0).toLocaleString('id-ID')}`;
        document.getElementById("shiftTxCount").textContent = `${d.shift_tx_count || 0} transaksi pengisian`;

        // Today Summary
        document.getElementById("todayVol").textContent = Number(d.today_liters || 0).toFixed(2);
        document.getElementById("todayCost").textContent = `${d.currency} ${Math.round(d.today_cost || 0).toLocaleString('id-ID')}`;
        document.getElementById("todayTxCount").textContent = `${d.today_tx_count || 0} transaksi`;
        if (d.today_date) document.getElementById("todayDateStr").textContent = d.today_date;

        // Clock & Valve
        document.getElementById("clockText").textContent = d.system_time || "Standby";
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
          // Enable stop button if flow is active or batch volume has been dispensed (supports manual nozzle mode)
          document.getElementById("btnStop").disabled = (!d.is_dispensing && Number(d.batch_liters) <= 0.005);
        }

        // Alarm
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

        // WiFi
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

    async function resetShift() {
      if (confirm("Reset total shift operator ini ke 0?")) {
        await fetch("/api/shift/reset", { method: "POST" });
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

    function printLastBatchReceipt() {
      if (!latestData || (!latestData.last_liters && !latestData.batch_liters)) {
        alert("Belum ada transaksi pengisian untuk dicetak.");
        return;
      }
      const vol = latestData.last_liters || latestData.batch_liters;
      const cost = latestData.last_cost || latestData.batch_cost;
      const timeStr = latestData.last_time || latestData.system_time;

      document.getElementById("rcptId").textContent = `#${String(latestData.last_id || latestData.history_count || 1).padStart(4, '0')}`;
      document.getElementById("rcptTime").textContent = timeStr;
      document.getElementById("rcptFluid").textContent = latestData.fluid_name || "Bio Solar";
      document.getElementById("rcptMode").textContent = "Transaksi Dispenser";
      document.getElementById("rcptVolume").textContent = Number(vol).toFixed(3);
      document.getElementById("rcptUnit").textContent = latestData.unit_name || "L";
      const pricePerL = vol > 0 ? Math.round(cost / vol) : Math.round(latestData.price_per_liter || 6800);
      document.getElementById("rcptPrice").textContent = `${latestData.currency || 'Rp'} ${pricePerL.toLocaleString('id-ID')}`;
      document.getElementById("rcptCost").textContent = `${latestData.currency || 'Rp'} ${Math.round(cost).toLocaleString('id-ID')}`;

      document.getElementById("receiptModal").style.display = "flex";
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

    async function saveMultiPointCalibration() {
      const pin = document.getElementById("cfgMpPin").value;
      if (!pin) {
        alert("Masukkan PIN Admin!");
        return;
      }
      const payload = {
        pin: pin,
        enabled: document.getElementById("cfgMpEnabled").checked,
        k_low: parseFloat(document.getElementById("cfgKLow").value),
        k_mid: parseFloat(document.getElementById("cfgKMid").value),
        k_high: parseFloat(document.getElementById("cfgKHigh").value)
      };
      try {
        const res = await fetch("/api/calibrate/multipoint", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(payload)
        });
        const d = await res.json();
        if (d.status === "ok") {
          alert("Kalibrasi 3-titik berhasil disimpan ke ESP32!");
          fetchData();
        } else {
          alert(d.error || "Gagal menyimpan kalibrasi.");
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

    function startOtaUpload() {
      const fileInput = document.getElementById("otaFileInput");
      if (!fileInput.files || fileInput.files.length === 0) {
        alert("Silakan pilih file firmware (.bin) terlebih dahulu!");
        return;
      }
      const file = fileInput.files[0];
      if (!file.name.endsWith(".bin")) {
        alert("File harus berformat .bin!");
        return;
      }

      if (!confirm(`Yakin ingin meng-upgrade firmware menggunakan file "${file.name}" (${(file.size / 1024).toFixed(1)} KB)?`)) {
        return;
      }

      const progressBg = document.getElementById("otaProgressBg");
      const progressFill = document.getElementById("otaProgressFill");
      const statusText = document.getElementById("otaStatusText");
      const btn = document.getElementById("btnOtaUpload");

      progressBg.style.display = "block";
      progressFill.style.width = "0%";
      statusText.textContent = "Mengunggah firmware... Harap jangan mematikan ESP32!";
      btn.disabled = true;

      const xhr = new XMLHttpRequest();
      xhr.open("POST", "/update", true);

      xhr.upload.onprogress = (e) => {
        if (e.lengthComputable) {
          const percent = Math.round((e.loaded / e.total) * 100);
          progressFill.style.width = percent + "%";
          statusText.textContent = `Mengunggah: ${percent}% (${Math.round(e.loaded/1024)} KB / ${Math.round(e.total/1024)} KB)`;
        }
      };

      xhr.onload = () => {
        if (xhr.status === 200) {
          statusText.innerHTML = "✅ <b>Update Berhasil!</b> ESP32 sedang restart... Halaman akan dimuat ulang dalam 6 detik.";
          progressFill.style.width = "100%";
          setTimeout(() => { window.location.reload(); }, 6000);
        } else {
          statusText.innerHTML = "❌ <b>Update Gagal!</b> Kode status: " + xhr.status;
          btn.disabled = false;
        }
      };

      xhr.onerror = () => {
        statusText.innerHTML = "❌ <b>Koneksi terputus saat mengunggah!</b>";
        btn.disabled = false;
      };

      const formData = new FormData();
      formData.append("update", file);
      xhr.send(formData);
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
