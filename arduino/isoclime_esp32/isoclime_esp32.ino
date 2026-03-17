/*
 * IsoClime ESP32 Sensor Node
 * ──────────────────────────────────────────────────────────────────────────
 * Serves live microclimate data over:
 *   HTTP  — GET http://<ip>/data   → JSON (polled by the dashboard)
 *   WS    — ws://<ip>/ws           → same JSON pushed every READ_INTERVAL_MS
 *
 * JSON format expected by the IsoClime Dashboard:
 *   {"z1":29.4,"z2":31.8,"z3":26.1,"ctrl_temp":28.7,"ctrl_hum":76}
 *
 * ── Default hardware wiring (4-sensor, 2-pin setup) ──────────────────────
 *
 *   ESP32 pin 4  ──── DHT22 data  (ambient control sensor)
 *   ESP32 pin 5  ──── DS18B20 × 3 data bus  (4.7 kΩ pull-up to 3.3 V)
 *   All sensors powered from 3.3 V / GND
 *
 *   ⚠️  SEEING 0.0 °C IN THE DASHBOARD?
 *       The sensor read failed — most common reasons:
 *         DS18B20 → missing 4.7 kΩ pull-up resistor between DATA and 3.3 V
 *         DHT22   → missing 10 kΩ pull-up resistor between DATA and 3.3 V,
 *                   or sampling too fast (needs ≥2 s between reads)
 *       Open Serial Monitor (115 200 baud) — failed sensors are logged with
 *       the exact reason so you can pinpoint the problem quickly.
 *
 *   DS18B20 physical order on the bus = Zone order (1 → 2 → 3).
 *   The first time you flash, open Serial Monitor (115 200 baud) and note
 *   the printed device addresses — that tells you which sensor is which zone.
 *
 * ── SINGLE_SENSOR_MODE ───────────────────────────────────────────────────
 *   Set  #define SINGLE_SENSOR_MODE  1  below to run with just ONE DHT22.
 *   Zone 2 and 3 will be offset ±2 °C from the DHT22 reading so the
 *   dashboard still shows distinct values while you source more sensors.
 *
 * ── Required libraries (Sketch → Manage Libraries) ───────────────────────
 *   • ESP Async WebServer  by ESP32Async   (ALSO install AsyncTCP by ESP32Async — same author, required companion)
 *     ⚠️  If you get "ESPAsyncWebServer.h: No such file or directory", this
 *         library is not installed — open Sketch → Manage Libraries, search
 *         "ESP Async WebServer", and install the one by ESP32Async.
 *     ⚠️  If you get "discards qualifiers", an old AsyncTCP (Me-No-Dev) is still
 *         installed. Delete it and install AsyncTCP by ESP32Async instead.
 *   • DHT sensor library   by Adafruit
 *   • Adafruit Unified Sensor  by Adafruit  (DHT dependency)
 *   • OneWire              by Jim Studt / Paul Stoffregen
 *   • DallasTemperature    by Miles Burton
 *
 * ── Board setup (first time only) ────────────────────────────────────────
 *   Arduino IDE → File → Preferences → Additional Boards Manager URLs:
 *     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *   Then: Tools → Board → Boards Manager → search "esp32" → Install
 *   Select board: Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
 */

// ═══════════════════════ CONFIGURATION ═══════════════════════════════════════

// 0 = full hardware (3× DS18B20 + 1× DHT22)  ← default
// 1 = single DHT22 only (synthetic zone offsets, no DS18B20)
#define SINGLE_SENSOR_MODE  0

// ── WiFi credentials ──────────────────────────────────────────────────────────
//
//  SCHOOL_WIFI  0 = use HOME WiFi credentials (default — use at home)
//  SCHOOL_WIFI  1 = use SCHOOL WiFi credentials + static IP 11.0.8.106
//
//  ★ HOW TO SWITCH WHEN YOU GET TO SCHOOL:
//    Step 1 — Change  #define SCHOOL_WIFI  0  →  #define SCHOOL_WIFI  1  (below)
//    Step 2 — Fill in your school's WiFi name + password in secrets.h (SCHOOL block).
//    Step 3 — Plug the ESP32 into a computer and click Upload in Arduino IDE.
//    Step 4 — Dashboard is at  http://11.0.8.106/  (fixed — never changes at school)
//
#define SCHOOL_WIFI  0

#if __has_include("secrets.h")
  // ★ RECOMMENDED: put your real credentials in secrets.h (same folder as this
  //   file).  secrets.h is in .gitignore so it is never pushed to GitHub.
  //   Copy arduino/isoclime_esp32/secrets.h.example → secrets.h and fill it in.
  //   secrets.h uses the same SCHOOL_WIFI toggle above, so you only need to
  //   change SCHOOL_WIFI 0 → 1 when you get to school — no other edits needed.
  #include "secrets.h"
#elif SCHOOL_WIFI == 0
  // ── Home WiFi ──────────────────────────────────────────────────────────────
  const char* WIFI_SSID = "YOUR_HOME_WIFI_SSID";     // ← your home network name
  const char* WIFI_PASS = "YOUR_HOME_WIFI_PASSWORD";  // ← your home password
#else
  // ── School WiFi ────────────────────────────────────────────────────────────
  const char* WIFI_SSID = "YOUR_SCHOOL_WIFI_SSID";    // ← school network name
  const char* WIFI_PASS = "YOUR_SCHOOL_WIFI_PASSWORD"; // ← school password (use "" if open/no password)
#endif

// Pin assignments
#define DHT_PIN       4    // DHT22 data pin
#define DHT_TYPE      DHT22
#define ONE_WIRE_BUS  5    // DS18B20 one-wire bus pin (all 3 sensors share this)

// How often (ms) to read sensors and push data to WebSocket clients.
// 2000 ms (2 seconds) is a good balance between responsiveness and sensor load.
// DHT22 minimum sample period is 2 s; DS18B20 is < 1 s at 12-bit resolution.
const unsigned long READ_INTERVAL_MS = 2000;

// ═════════════════════════════════════════════════════════════════════════════

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

#if SINGLE_SENSOR_MODE == 0
  #include <OneWire.h>
  #include <DallasTemperature.h>
#endif

// ── Full IsoClime dashboard HTML ──────────────────────────────────────────────
// Served at http://<ip>/ — open this URL in Safari on your iPad (or any browser)
// while on the same WiFi as the ESP32.  Everything is self-contained (no CDN),
// so it works perfectly on a school/home network with or without internet access.
//
// Features: IsoClime logo · live clock · zone cards with sparklines · rolling
// multi-zone line chart · humidity gauge · comparison bar chart · zone map.
// All values update automatically every 2 s from GET /data on this device.
static const char DASHBOARD_HTML[] PROGMEM = R"html(<!doctype html><html><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>IsoClime Live</title><style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#f0fdfb;color:#0f2f28;min-height:100vh}
.wrap{max-width:1200px;margin:0 auto;padding:0 14px 28px}
.card{background:rgba(255,255,255,.85);border:1px solid rgba(52,211,153,.2);border-radius:16px;box-shadow:0 2px 10px rgba(0,0,0,.05)}
header{background:rgba(255,255,255,.95);border-bottom:1px solid rgba(52,211,153,.18);padding:12px 18px;display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:10px;position:sticky;top:0;z-index:10;margin-bottom:14px}
.logo{display:flex;align-items:center;gap:11px}
.logo-icon{width:38px;height:38px;background:linear-gradient(135deg,#064e3b,#065f46);border-radius:10px;display:flex;align-items:center;justify-content:center;box-shadow:0 0 14px rgba(52,211,153,.25)}
.logo-name{font-size:19px;font-weight:700;background:linear-gradient(135deg,#059669,#0d9488,#0891b2);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text}
.logo-sub{font-size:10px;color:#047857;margin-top:1px}
.lbadge{display:flex;align-items:center;gap:6px;background:rgba(52,211,153,.12);border:1px solid rgba(52,211,153,.3);border-radius:99px;padding:5px 12px}
.dot{width:7px;height:7px;border-radius:50%;background:#16a34a;box-shadow:0 0 6px #16a34a;animation:pp 1.8s ease-in-out infinite}
@keyframes pp{0%,100%{opacity:1;transform:scale(1)}50%{opacity:.35;transform:scale(1.4)}}
.btxt{font-size:11px;font-weight:700;color:#047857;font-family:monospace;letter-spacing:.06em}
.clk{font-size:11px;color:#0d7377;font-family:monospace}
.zrow{display:grid;grid-template-columns:repeat(4,1fr);gap:12px;margin-bottom:12px}
@media(max-width:680px){.zrow{grid-template-columns:repeat(2,1fr)}}
.zc{padding:15px}
.zlbl{font-size:10px;text-transform:uppercase;letter-spacing:.1em;color:#0d7377;font-weight:600;opacity:.8}
.zname{font-size:12px;font-weight:600;color:#047857;margin-top:2px;margin-bottom:8px}
.ztmp{font-size:36px;font-weight:700;color:#0f2f28;line-height:1;font-family:monospace}
.zdeg{font-size:15px;color:#34d399}
.hbadge{display:inline-block;padding:2px 8px;border-radius:99px;font-size:10px;font-weight:700;font-family:monospace;margin:6px 0 2px}
.ztrend{font-size:10px;color:#0d7377;font-family:monospace;opacity:.8}
.crow{display:flex;justify-content:space-between;align-items:center;background:rgba(52,211,153,.06);border:1px solid rgba(52,211,153,.12);border-radius:9px;padding:7px 11px;margin-bottom:7px}
.clabel{font-size:11px;color:#0d9488}
.cval{font-size:17px;font-weight:700;font-family:monospace}
.hbw{height:5px;border-radius:99px;background:rgba(20,184,166,.1);overflow:hidden}
.hbar{height:100%;border-radius:99px;background:linear-gradient(90deg,#14b8a6,#06b6d4);transition:width 1s ease}
.crow2{display:grid;grid-template-columns:1fr 186px 186px;gap:12px;margin-bottom:12px}
@media(max-width:680px){.crow2{grid-template-columns:1fr}}
.cc{padding:16px}
.cht{font-size:13px;font-weight:700;color:#0f2f28;margin-bottom:2px}
.chs{font-size:10px;color:#0d7377;opacity:.7;margin-bottom:8px}
.leg{display:flex;gap:12px;flex-wrap:wrap;margin-bottom:6px}
.li{display:flex;align-items:center;gap:4px;font-size:10px;color:#0d7377}
.ld{width:14px;height:3px;border-radius:2px}
.mapc{padding:16px;margin-bottom:12px}
.mapinner{position:relative;background:rgba(6,20,14,.55);border:1px solid rgba(52,211,153,.1);border-radius:10px;overflow:hidden;height:156px}
.znode{position:absolute;display:flex;flex-direction:column;align-items:center;gap:4px;transform:translate(-50%,-50%)}
.znc{width:50px;height:50px;border-radius:50%;display:flex;align-items:center;justify-content:center;font-size:12px;font-weight:700;border:2px solid;font-family:monospace}
.znl{font-size:9px;font-weight:600;color:#e0f5ee;text-align:center;line-height:1.3}
.znt{font-size:9px;font-family:monospace;text-align:center}
.stbar{text-align:center;margin-top:4px;font-size:11px;color:#0d9488;font-family:monospace;padding-bottom:10px}
@keyframes flsh{0%{color:#34d399}100%{color:inherit}}
.fl{animation:flsh .5s ease}
</style></head><body>
<header>
 <div class="logo">
  <div class="logo-icon">
   <svg width="22" height="22" viewBox="0 0 26 26" fill="none">
    <path d="M13 3C13 3 4 8.5 4 15.5C4 20.19 8.03 24 13 24C17.97 24 22 20.19 22 15.5C22 8.5 13 3 13 3Z" fill="#34d399" opacity=".9"/>
    <path d="M13 24V12" stroke="#064e3b" stroke-width="1.8" stroke-linecap="round"/>
    <path d="M13 17C13 17 9 14 7 11" stroke="#064e3b" stroke-width="1.4" stroke-linecap="round" opacity=".7"/>
   </svg>
  </div>
  <div>
   <div class="logo-name">IsoClime</div>
   <div class="logo-sub">Live Microclimate Sensor Network</div>
  </div>
 </div>
 <div style="display:flex;align-items:center;gap:12px">
  <div class="lbadge"><span class="dot"></span><span class="btxt">LIVE</span></div>
  <span id="clk" class="clk">--:--:--</span>
 </div>
</header>
<div class="wrap">
 <!-- ── Zone cards ── -->
 <div class="zrow">
  <div class="card zc">
   <div class="zlbl">Zone 1</div><div class="zname">Urban Road</div>
   <div class="ztmp"><span id="z1t">--</span><span class="zdeg">&deg;C</span></div>
   <span id="z1b" class="hbadge">--</span>&nbsp;<span id="z1r" class="ztrend"></span>
   <svg width="100%" height="34" viewBox="0 0 160 34" preserveAspectRatio="none" style="display:block;margin-top:6px">
    <defs><linearGradient id="g1" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#34d399" stop-opacity=".3"/><stop offset="100%" stop-color="#34d399" stop-opacity="0"/></linearGradient></defs>
    <path id="sf1" fill="url(#g1)"/><path id="sl1" stroke="#34d399" stroke-width="2" fill="none"/>
   </svg>
  </div>
  <div class="card zc">
   <div class="zlbl">Zone 2</div><div class="zname">Alum. Roof Houses</div>
   <div class="ztmp"><span id="z2t">--</span><span class="zdeg" style="color:#14b8a6">&deg;C</span></div>
   <span id="z2b" class="hbadge">--</span>&nbsp;<span id="z2r" class="ztrend"></span>
   <svg width="100%" height="34" viewBox="0 0 160 34" preserveAspectRatio="none" style="display:block;margin-top:6px">
    <defs><linearGradient id="g2" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#14b8a6" stop-opacity=".3"/><stop offset="100%" stop-color="#14b8a6" stop-opacity="0"/></linearGradient></defs>
    <path id="sf2" fill="url(#g2)"/><path id="sl2" stroke="#14b8a6" stroke-width="2" fill="none"/>
   </svg>
  </div>
  <div class="card zc">
   <div class="zlbl">Zone 3</div><div class="zname">Tree-Covered Area</div>
   <div class="ztmp"><span id="z3t">--</span><span class="zdeg" style="color:#06b6d4">&deg;C</span></div>
   <span id="z3b" class="hbadge">--</span>&nbsp;<span id="z3r" class="ztrend"></span>
   <svg width="100%" height="34" viewBox="0 0 160 34" preserveAspectRatio="none" style="display:block;margin-top:6px">
    <defs><linearGradient id="g3" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#06b6d4" stop-opacity=".3"/><stop offset="100%" stop-color="#06b6d4" stop-opacity="0"/></linearGradient></defs>
    <path id="sf3" fill="url(#g3)"/><path id="sl3" stroke="#06b6d4" stroke-width="2" fill="none"/>
   </svg>
  </div>
  <div class="card zc">
   <div class="zlbl">Control Sensor</div><div class="zname">DHT22 &mdash; Ambient</div>
   <div class="crow"><span class="clabel">&#x1F321; Temperature</span><span class="cval"><span id="ctt">--</span><span style="font-size:12px;color:#34d399">&deg;C</span></span></div>
   <div class="crow"><span class="clabel">&#x1F4A7; Humidity</span><span class="cval"><span id="cth">--</span><span style="font-size:12px;color:#14b8a6">%</span></span></div>
   <div>
    <div style="display:flex;justify-content:space-between;margin-bottom:3px">
     <span style="font-size:10px;color:#0d9488">Relative Humidity</span>
     <span id="hp" style="font-size:10px;font-family:monospace;color:#0d7377">--%</span>
    </div>
    <div class="hbw"><div id="hb" class="hbar" style="width:0"></div></div>
   </div>
  </div>
 </div>
 <!-- ── Charts row ── -->
 <div class="crow2">
  <!-- Multi-zone line chart -->
  <div class="card cc">
   <div class="cht">Temperature Over Time</div>
   <div class="chs">All zones &mdash; rolling last 11 readings</div>
   <div class="leg">
    <div class="li"><div class="ld" style="background:#34d399"></div>Z1 Urban Road</div>
    <div class="li"><div class="ld" style="background:#14b8a6"></div>Z2 Alum. Roofs</div>
    <div class="li"><div class="ld" style="background:#06b6d4"></div>Z3 Tree Cover</div>
   </div>
   <svg id="lc" viewBox="0 0 520 140" preserveAspectRatio="xMidYMid meet" style="width:100%;height:140px;overflow:visible">
    <defs>
     <linearGradient id="la1g" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#34d399" stop-opacity=".13"/><stop offset="100%" stop-color="#34d399" stop-opacity="0"/></linearGradient>
     <linearGradient id="la2g" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#14b8a6" stop-opacity=".13"/><stop offset="100%" stop-color="#14b8a6" stop-opacity="0"/></linearGradient>
     <linearGradient id="la3g" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#06b6d4" stop-opacity=".13"/><stop offset="100%" stop-color="#06b6d4" stop-opacity="0"/></linearGradient>
    </defs>
    <line x1="0" y1="10"  x2="520" y2="10"  stroke="rgba(100,220,170,.06)" stroke-width="1"/>
    <line x1="0" y1="50"  x2="520" y2="50"  stroke="rgba(100,220,170,.06)" stroke-width="1"/>
    <line x1="0" y1="90"  x2="520" y2="90"  stroke="rgba(100,220,170,.06)" stroke-width="1"/>
    <line x1="0" y1="130" x2="520" y2="130" stroke="rgba(100,220,170,.06)" stroke-width="1"/>
    <text x="-3" y="14"  fill="#0d7377" font-size="9" text-anchor="end" font-family="monospace">34&#176;</text>
    <text x="-3" y="54"  fill="#0d7377" font-size="9" text-anchor="end" font-family="monospace">31&#176;</text>
    <text x="-3" y="94"  fill="#0d7377" font-size="9" text-anchor="end" font-family="monospace">28&#176;</text>
    <text x="-3" y="134" fill="#0d7377" font-size="9" text-anchor="end" font-family="monospace">25&#176;</text>
    <path id="la1" fill="url(#la1g)"/><path id="la2" fill="url(#la2g)"/><path id="la3" fill="url(#la3g)"/>
    <path id="ll1" stroke="#34d399" stroke-width="2.2" fill="none"/>
    <path id="ll2" stroke="#14b8a6" stroke-width="2.2" fill="none"/>
    <path id="ll3" stroke="#06b6d4" stroke-width="2.2" fill="none"/>
    <circle id="ld1" cx="520" cy="70" r="4" fill="#34d399"/>
    <circle id="ld2" cx="520" cy="70" r="4" fill="#14b8a6"/>
    <circle id="ld3" cx="520" cy="70" r="4" fill="#06b6d4"/>
   </svg>
  </div>
  <!-- Humidity gauge -->
  <div class="card cc">
   <div class="cht">Humidity Gauge</div>
   <div class="chs">Ambient &mdash; DHT22</div>
   <div style="display:flex;flex-direction:column;align-items:center">
    <svg width="138" height="138" viewBox="0 0 150 150">
     <defs><linearGradient id="gg" x1="0%" y1="0%" x2="100%" y2="0%"><stop offset="0%" stop-color="#14b8a6"/><stop offset="100%" stop-color="#06b6d4"/></linearGradient></defs>
     <circle cx="75" cy="75" r="58" fill="none" stroke="rgba(100,220,170,.12)" stroke-width="12" stroke-dasharray="290 365" stroke-dashoffset="-37" stroke-linecap="round" transform="rotate(90 75 75)"/>
     <circle id="gf" cx="75" cy="75" r="58" fill="none" stroke="url(#gg)" stroke-width="12" stroke-dasharray="0 365" stroke-dashoffset="-37" stroke-linecap="round" transform="rotate(90 75 75)" style="transition:stroke-dasharray 1s ease;filter:drop-shadow(0 0 5px #14b8a6)"/>
     <text id="gv" x="75" y="68" text-anchor="middle" fill="#0f2f28" font-size="26" font-weight="700" font-family="monospace">--</text>
     <text x="75" y="85" text-anchor="middle" fill="#0d9488" font-size="11" font-family="sans-serif">%RH</text>
     <text x="75" y="104" text-anchor="middle" fill="#0d7377" font-size="8" font-family="sans-serif">RELATIVE HUMIDITY</text>
    </svg>
    <div style="display:flex;justify-content:space-between;width:138px;margin-top:2px">
     <span style="font-size:9px;color:#0d7377;font-family:monospace;opacity:.5">0%</span>
     <span style="font-size:9px;color:#0d7377;font-family:monospace;opacity:.5">100%</span>
    </div>
    <div style="margin-top:10px;display:flex;align-items:center;justify-content:space-between;width:138px">
     <span style="font-size:10px;color:#047857">Comfort:</span>
     <span id="cmf" style="font-size:10px;font-family:monospace;font-weight:700;color:#fbbf24">--</span>
    </div>
   </div>
  </div>
  <!-- Bar chart -->
  <div class="card cc">
   <div class="cht">Zone Comparison</div>
   <div class="chs">Current temperature by zone</div>
   <div style="display:flex;align-items:flex-end;justify-content:space-around;height:106px;gap:8px;margin-bottom:10px">
    <div style="display:flex;flex-direction:column;align-items:center;flex:1">
     <span id="bv1" style="font-size:11px;font-weight:700;font-family:monospace;margin-bottom:5px">--&#176;</span>
     <div style="width:100%;height:76px;display:flex;align-items:flex-end">
      <div id="b1" style="width:100%;height:50%;background:linear-gradient(180deg,#34d399,rgba(52,211,153,.3));border-radius:5px 5px 2px 2px;transition:height .8s ease;box-shadow:0 0 8px rgba(52,211,153,.25)"></div>
     </div>
     <span style="font-size:9px;color:#0d7377;text-align:center;margin-top:6px">Urban<br>Road</span>
    </div>
    <div style="display:flex;flex-direction:column;align-items:center;flex:1">
     <span id="bv2" style="font-size:11px;font-weight:700;font-family:monospace;margin-bottom:5px">--&#176;</span>
     <div style="width:100%;height:76px;display:flex;align-items:flex-end">
      <div id="b2" style="width:100%;height:50%;background:linear-gradient(180deg,#14b8a6,rgba(20,184,166,.3));border-radius:5px 5px 2px 2px;transition:height .8s ease;box-shadow:0 0 8px rgba(20,184,166,.25)"></div>
     </div>
     <span style="font-size:9px;color:#0d7377;text-align:center;margin-top:6px">Alum.<br>Roofs</span>
    </div>
    <div style="display:flex;flex-direction:column;align-items:center;flex:1">
     <span id="bv3" style="font-size:11px;font-weight:700;font-family:monospace;margin-bottom:5px">--&#176;</span>
     <div style="width:100%;height:76px;display:flex;align-items:flex-end">
      <div id="b3" style="width:100%;height:50%;background:linear-gradient(180deg,#06b6d4,rgba(6,182,212,.3));border-radius:5px 5px 2px 2px;transition:height .8s ease;box-shadow:0 0 8px rgba(6,182,212,.25)"></div>
     </div>
     <span style="font-size:9px;color:#0d7377;text-align:center;margin-top:6px">Tree<br>Cover</span>
    </div>
   </div>
   <div style="display:flex;flex-direction:column;gap:4px;font-size:10px;color:#0d7377">
    <div style="display:flex;align-items:center;gap:5px"><div style="width:8px;height:8px;border-radius:50%;background:#ef4444;box-shadow:0 0 5px #ef4444"></div><span id="hi2">--</span></div>
    <div style="display:flex;align-items:center;gap:5px"><div style="width:8px;height:8px;border-radius:50%;background:#f59e0b;box-shadow:0 0 5px #f59e0b"></div><span id="hi1">--</span></div>
    <div style="display:flex;align-items:center;gap:5px"><div style="width:8px;height:8px;border-radius:50%;background:#34d399;box-shadow:0 0 5px #34d399"></div><span>Cool &mdash; Tree Cover (baseline)</span></div>
   </div>
  </div>
 </div>
 <!-- ── Zone map ── -->
 <div class="card mapc">
  <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:12px;flex-wrap:wrap;gap:8px">
   <div>
    <div class="cht">Sensor Network Map</div>
    <div class="chs">Spatial distribution of monitoring zones</div>
   </div>
   <div style="font-size:10px;font-family:monospace;color:#047857;background:rgba(52,211,153,.08);border:1px solid rgba(52,211,153,.2);border-radius:99px;padding:4px 10px">3 Nodes Active</div>
  </div>
  <div class="mapinner">
   <svg width="100%" height="100%" style="position:absolute;inset:0" preserveAspectRatio="xMidYMid meet">
    <defs><pattern id="grid" width="40" height="40" patternUnits="userSpaceOnUse"><path d="M 40 0 L 0 0 0 40" fill="none" stroke="rgba(52,211,153,.05)" stroke-width="1"/></pattern></defs>
    <rect width="100%" height="100%" fill="url(#grid)"/>
    <line x1="22%" y1="52%" x2="54%" y2="60%" stroke="rgba(52,211,153,.2)" stroke-width="1" stroke-dasharray="4,3"/>
    <line x1="54%" y1="60%" x2="79%" y2="44%" stroke="rgba(52,211,153,.2)" stroke-width="1" stroke-dasharray="4,3"/>
   </svg>
   <div class="znode" style="left:22%;top:52%">
    <div class="znc" style="background:rgba(52,211,153,.1);border-color:#34d399;color:#34d399">Z1</div>
    <div class="znl">Urban Road</div><div id="mz1" class="znt" style="color:#34d399">--&#176;C</div>
   </div>
   <div class="znode" style="left:54%;top:60%">
    <div class="znc" style="background:rgba(239,68,68,.1);border-color:#f87171;color:#f87171">Z2</div>
    <div class="znl">Alum. Roofs</div><div id="mz2" class="znt" style="color:#f87171">--&#176;C</div>
   </div>
   <div class="znode" style="left:79%;top:44%">
    <div class="znc" style="background:rgba(6,182,212,.1);border-color:#06b6d4;color:#06b6d4">Z3</div>
    <div class="znl">Tree Cover</div><div id="mz3" class="znt" style="color:#06b6d4">--&#176;C</div>
   </div>
   <div style="position:absolute;bottom:10px;right:12px;opacity:.3;font-size:9px;color:#6ee7b7;font-family:monospace;text-align:center">N<br><span style="font-size:14px">&#8853;</span></div>
   <div style="position:absolute;bottom:12px;left:12px;display:flex;align-items:center;gap:4px;opacity:.35">
    <div style="width:22px;height:2px;background:#34d399;border-radius:1px"></div>
    <span style="font-size:8px;color:#6ee7b7;font-family:monospace">50m</span>
   </div>
  </div>
 </div>
 <div id="st" class="stbar">Connecting&hellip;</div>
</div>
<script>
var HIST=11,CW=520,CH=140,CBL=140;
var hist={z1:[],z2:[],z3:[]};
function tY(t){var mn=24,mx=36,c=Math.max(mn,Math.min(mx,t));return(CH-10-((c-mn)/(mx-mn))*(CH-20)).toFixed(1);}
function mkLine(pts){
  if(pts.length<2)return '';
  var s=CW/(pts.length-1);
  return pts.map(function(v,i){return(i===0?'M':'L')+(i*s).toFixed(1)+','+tY(v);}).join(' ');
}
function mkArea(pts){
  var ln=mkLine(pts);if(!ln)return '';
  var lx=((pts.length-1)*(CW/(pts.length-1))).toFixed(1);
  return ln+' L'+lx+','+CBL+' L0,'+CBL+'Z';
}
function mkSpark(h){
  var sw=160,sh=34,n=h.length;if(n<2)return{l:'',f:''};
  var s=sw/(n-1);
  var mn=h[0],mx=h[0];
  for(var i=1;i<n;i++){if(h[i]<mn)mn=h[i];if(h[i]>mx)mx=h[i];}
  var rng=mx-mn||1;
  function yOf(v){return((1-(v-mn)/rng)*(sh-8)+4).toFixed(1);}
  var pts=h.map(function(v,i){return[(i*s).toFixed(1),yOf(v)];});
  var l=pts.map(function(p,i){return(i===0?'M':'L')+p[0]+','+p[1];}).join(' ');
  var f=l+' L'+pts[n-1][0]+','+sh+' L0,'+sh+'Z';
  return{l:l,f:f};
}
function mkBadge(t,base){
  var d=t-base;
  if(d<-0.5)return{lbl:'COOL',    bg:'rgba(52,211,153,.12)', col:'#059669',bdr:'rgba(52,211,153,.25)'};
  if(d< 0.5)return{lbl:'NORMAL',  bg:'rgba(52,211,153,.08)', col:'#0d9488',bdr:'rgba(52,211,153,.2)'};
  if(d<   2)return{lbl:'WARM',    bg:'rgba(245,158,11,.12)', col:'#d97706',bdr:'rgba(245,158,11,.3)'};
  if(d<   4)return{lbl:'HIGH',    bg:'rgba(239,68,68,.12)',  col:'#dc2626',bdr:'rgba(239,68,68,.25)'};
  return           {lbl:'ELEVATED',bg:'rgba(239,68,68,.18)', col:'#f87171',bdr:'rgba(239,68,68,.3)'};
}
function mkTrend(h){
  if(h.length<2)return '';
  var r=h[h.length-1]-h[h.length-2];
  return(r>0?'▲':'▼')+' '+(r>=0?'+':'')+r.toFixed(1)+'°C/rd';
}
function setTxt(id,v){
  var el=document.getElementById(id);if(!el)return;
  el.textContent=v;el.className='';void el.offsetWidth;el.className='fl';
}
function applyData(d){
  // null = sensor failed (check wiring / pull-up resistor — see Serial Monitor)
  function ok(v){return v!==null&&v!==undefined;}
  var z1t=ok(d.z1)?d.z1:null,z2t=ok(d.z2)?d.z2:null,z3t=ok(d.z3)?d.z3:null;
  var base=z3t!==null?z3t:(z1t!==null?z1t:(z2t!==null?z2t:null));
  var zones=[
    {t:z1t,ti:'z1t',bi:'z1b',ri:'z1r',hk:'z1',sl:'sl1',sf:'sf1'},
    {t:z2t,ti:'z2t',bi:'z2b',ri:'z2r',hk:'z2',sl:'sl2',sf:'sf2'},
    {t:z3t,ti:'z3t',bi:'z3b',ri:'z3r',hk:'z3',sl:'sl3',sf:'sf3'}
  ];
  zones.forEach(function(z){
    if(z.t!==null){
      hist[z.hk].push(z.t);
      if(hist[z.hk].length>HIST)hist[z.hk].shift();
      setTxt(z.ti,z.t.toFixed(1));
      var b=mkBadge(z.t,base!==null?base:z.t);
      var bel=document.getElementById(z.bi);
      if(bel){bel.textContent=b.lbl;bel.style.background=b.bg;bel.style.color=b.col;bel.style.border='1px solid '+b.bdr;}
    } else {
      setTxt(z.ti,'--');
      var bel=document.getElementById(z.bi);
      if(bel){bel.textContent='ERR';bel.style.background='rgba(239,68,68,.12)';bel.style.color='#dc2626';bel.style.border='1px solid rgba(239,68,68,.3)';}
    }
    var rel=document.getElementById(z.ri);if(rel)rel.textContent=mkTrend(hist[z.hk]);
    var sp=mkSpark(hist[z.hk]);
    var sle=document.getElementById(z.sl),sfe=document.getElementById(z.sf);
    if(sle)sle.setAttribute('d',sp.l);if(sfe)sfe.setAttribute('d',sp.f);
  });
  var ct=ok(d.ctrl_temp)?d.ctrl_temp:null,ch=ok(d.ctrl_hum)?d.ctrl_hum:null;
  setTxt('ctt',ct!==null?ct.toFixed(1):'--');
  setTxt('cth',ch!==null?String(Math.round(ch)):'--');
  var hpEl=document.getElementById('hp');if(hpEl)hpEl.textContent=ch!==null?Math.round(ch)+'%':'--%';
  var hbEl=document.getElementById('hb');if(hbEl)hbEl.style.width=ch!==null?Math.min(100,ch).toFixed(1)+'%':'0%';
  var gfEl=document.getElementById('gf'),gvEl=document.getElementById('gv');
  if(gfEl)gfEl.setAttribute('stroke-dasharray',ch!==null?(290*(ch/100)).toFixed(1)+' 365':'0 365');
  if(gvEl)gvEl.textContent=ch!==null?String(Math.round(ch)):'--';
  var cmfEl=document.getElementById('cmf');
  if(cmfEl){
    if(ch===null)cmfEl.textContent='--';
    else if(ch<40)cmfEl.textContent='DRY';
    else if(ch<60)cmfEl.textContent='COMFORT';
    else if(ch<75)cmfEl.textContent='HUMID';
    else cmfEl.textContent='VERY HUMID';
  }
  var validZ=[z1t,z2t,z3t].filter(function(v){return v!==null;});
  if(validZ.length){
    var mx=Math.max.apply(null,validZ.concat([30])),mn=Math.min.apply(null,validZ.concat([24]));
    function bp(v){return v!==null?Math.max(15,Math.round(((v-mn)/(mx-mn||1))*75+15)):15;}
    var b1=document.getElementById('b1'),b2=document.getElementById('b2'),b3=document.getElementById('b3');
    if(b1)b1.style.height=bp(z1t)+'%';if(b2)b2.style.height=bp(z2t)+'%';if(b3)b3.style.height=bp(z3t)+'%';
    setTxt('bv1',z1t!==null?z1t.toFixed(1)+'°':'--');setTxt('bv2',z2t!==null?z2t.toFixed(1)+'°':'--');setTxt('bv3',z3t!==null?z3t.toFixed(1)+'°':'--');
    var hi1El=document.getElementById('hi1'),hi2El=document.getElementById('hi2');
    if(hi2El)hi2El.textContent=z2t!==null&&z3t!==null?'Elevated — Alum. Roofs (+'+(z2t-z3t).toFixed(1)+'°C)':'Alum. Roofs — no data';
    if(hi1El)hi1El.textContent=z1t!==null&&z3t!==null?'Warm — Urban Road (+'+(z1t-z3t).toFixed(1)+'°C)':'Urban Road — no data';
  }
  var mps=[['mz1',z1t],['mz2',z2t],['mz3',z3t]];
  mps.forEach(function(p){var el=document.getElementById(p[0]);if(el)el.textContent=p[1]!==null?p[1].toFixed(1)+'°C':'--°C';});
  ['z1','z2','z3'].forEach(function(k,i){
    var h=hist[k];if(h.length<2)return;
    var n=i+1;
    var ll=document.getElementById('ll'+n),la=document.getElementById('la'+n),ld=document.getElementById('ld'+n);
    if(ll)ll.setAttribute('d',mkLine(h));
    if(la)la.setAttribute('d',mkArea(h));
    if(ld){ld.setAttribute('cx','520');ld.setAttribute('cy',tY(h[h.length-1]));}
  });
  var stEl=document.getElementById('st');
  if(stEl){
    var n=new Date();
    var ts=[n.getHours(),n.getMinutes(),n.getSeconds()].map(function(v){return String(v).padStart(2,'0');}).join(':');
    stEl.innerHTML='<span style="color:#16a34a">&#9679;</span> Live &mdash; '+ts;
  }
}
function tick(){
  var n=new Date(),el=document.getElementById('clk');
  if(el)el.textContent=[n.getHours(),n.getMinutes(),n.getSeconds()].map(function(v){return String(v).padStart(2,'0');}).join(':');
}
tick();setInterval(tick,1000);
function upd(){
  fetch('/data').then(function(r){return r.json();}).then(applyData).catch(function(){
    var el=document.getElementById('st');
    if(el)el.innerHTML='<span style="color:#dc2626">&#9888; Connection lost &mdash; retrying&hellip;</span>';
  });
}
upd();setInterval(upd,2000);
</script></body></html>)html";

// ── Sensor objects ────────────────────────────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);

#if SINGLE_SENSOR_MODE == 0
  OneWire       oneWire(ONE_WIRE_BUS);
  DallasTemperature ds18(&oneWire);
#endif

// ── Web server + WebSocket ────────────────────────────────────────────────────
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ── Latest readings (written by readSensors, read by HTTP handler) ────────────
float z1 = NAN, z2 = NAN, z3 = NAN;
float ctrlTemp = NAN, ctrlHum = NAN;
unsigned long lastRead = 0;

// ─────────────────────────────────────────────────────────────────────────────
// Build the JSON payload
// Failed sensors emit JSON null (not 0) so the dashboard shows "--" rather
// than a misleading 0.0 °C reading.
// ─────────────────────────────────────────────────────────────────────────────
String buildJson() {
  // Helper: return a numeric string (1 dp) or "null" for a failed reading
  auto jf = [](float v, int dp) -> String {
    if (isnan(v)) return "null";
    char tmp[16];
    if (dp == 0) snprintf(tmp, sizeof(tmp), "%.0f", v);
    else         snprintf(tmp, sizeof(tmp), "%.1f", v);
    return String(tmp);
  };
  String s = "{";
  s += "\"z1\":"       + jf(z1,       1) + ",";
  s += "\"z2\":"       + jf(z2,       1) + ",";
  s += "\"z3\":"       + jf(z3,       1) + ",";
  s += "\"ctrl_temp\":" + jf(ctrlTemp, 1) + ",";
  s += "\"ctrl_hum\":"  + jf(ctrlHum,  0);
  s += "}";
  return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// Read all sensors and store in the global floats
// ─────────────────────────────────────────────────────────────────────────────
void readSensors() {
  // Control sensor (DHT22) ─────────────────────────────────────────────────
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) {
    ctrlTemp = t;
  } else {
    Serial.println("[WARN] DHT22 temperature read failed — check wiring and");
    Serial.println("       10 kΩ pull-up on DATA pin. Dashboard will show '--'.");
  }
  if (!isnan(h)) {
    ctrlHum = h;
  } else {
    Serial.println("[WARN] DHT22 humidity read failed — same cause as above.");
  }

#if SINGLE_SENSOR_MODE
  // Use the DHT22 reading for all three zones with small synthetic offsets
  // so the dashboard shows plausible variation until real sensors are added
  if (!isnan(ctrlTemp)) {
    z1 = ctrlTemp + 0.7f;    // Zone 1 slightly warmer (e.g. urban road)
    z2 = ctrlTemp + 3.1f;    // Zone 2 hottest       (e.g. aluminium roofs)
    z3 = ctrlTemp - 1.3f;    // Zone 3 coolest       (e.g. tree cover)
  }
#else
  // Zone sensors (DS18B20 one-wire) ─────────────────────────────────────────
  ds18.requestTemperatures();   // blocks ~750 ms with default 12-bit resolution
  float r0 = ds18.getTempCByIndex(0);
  float r1 = ds18.getTempCByIndex(1);
  float r2 = ds18.getTempCByIndex(2);
  // DallasTemperature returns -127 on read error — treat as NaN
  if (r0 > -100) {
    z1 = r0;
  } else {
    Serial.println("[WARN] Zone 1 DS18B20 read failed (-127 °C) — common causes:");
    Serial.println("       • Missing 4.7 kΩ pull-up resistor (DATA → 3.3 V)");
    Serial.println("       • Loose or broken wire on the one-wire bus");
    Serial.println("       • Sensor not powered (check 3.3 V / GND)");
    Serial.println("       Dashboard will show '--' for Zone 1.");
  }
  if (r1 > -100) {
    z2 = r1;
  } else {
    Serial.println("[WARN] Zone 2 DS18B20 read failed (-127 °C) — see Zone 1 note above.");
    Serial.println("       Dashboard will show '--' for Zone 2.");
  }
  if (r2 > -100) {
    z3 = r2;
  } else {
    Serial.println("[WARN] Zone 3 DS18B20 read failed (-127 °C) — see Zone 1 note above.");
    Serial.println("       Dashboard will show '--' for Zone 3.");
  }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// WebSocket event handler
// ─────────────────────────────────────────────────────────────────────────────
void onWsEvent(AsyncWebSocket* srv, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("[WS] Client #%u connected (%s)\n",
                  client->id(), client->remoteIP().toString().c_str());
    // Send current reading immediately so the dashboard doesn't wait
    client->text(buildJson());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("[WS] Client #%u disconnected\n", client->id());
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// setup
// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);  // ← Serial Monitor baud rate must also be 115200 (Tools → Serial Monitor → bottom-right dropdown)
  delay(500);            // give the USB-serial link time to open so the first lines aren't lost
  Serial.println("\n=== IsoClime ESP32 Sensor Node ===");

  // Start sensors ────────────────────────────────────────────────────────────
  dht.begin();
#if SINGLE_SENSOR_MODE == 0
  ds18.begin();
  uint8_t count = ds18.getDeviceCount();
  Serial.printf("[Sensors] DS18B20 devices found: %d\n", count);
  if (count < 3) {
    Serial.println("[Sensors] WARNING: fewer than 3 DS18B20 sensors detected!");
    Serial.println("           Check wiring: data pin, 4.7 kΩ pull-up to 3.3 V, and power.");
    Serial.println("           If running with fewer sensors intentionally, set SINGLE_SENSOR_MODE 1.");
  }
  // Print device addresses for reference
  DeviceAddress addr;
  for (uint8_t i = 0; i < count; i++) {
    if (ds18.getAddress(addr, i)) {
      Serial.printf("[Sensors] Zone %d address: ", i + 1);
      for (uint8_t b = 0; b < 8; b++) Serial.printf("%02X", addr[b]);
      Serial.println();
    }
  }
#else
  Serial.println("[Sensors] SINGLE_SENSOR_MODE — one DHT22, synthetic zone offsets");
#endif

  // Initial sensor read so the first HTTP response has real data ─────────────
  readSensors();
  lastRead = millis();

  // Connect to WiFi ──────────────────────────────────────────────────────────
  WiFi.mode(WIFI_STA);
#ifdef WIFI_STATIC_IP
  // School network: assign a fixed IP so the dashboard URL never changes.
  WiFi.config(WIFI_LOCAL_IP, WIFI_GATEWAY, WIFI_SUBNET, WIFI_DNS1, WIFI_DNS2);
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[WiFi] Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.printf("\n[WiFi] Connected!  IP address: %s\n",
                WiFi.localIP().toString().c_str());
  Serial.println("[WiFi] Enter that IP in the IsoClime Dashboard → Connect");

  // ── HTTP routes ────────────────────────────────────────────────────────────

  // GET /data — returns latest sensor JSON
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest* req) {
    AsyncWebServerResponse* res =
      req->beginResponse(200, "application/json", buildJson());
    // CORS: allow the dashboard to fetch data regardless of where it is hosted
    // (GitHub Pages, local file, or any other origin). The ESP32 is a local device
    // on a private network — wildcard origin is appropriate here.
    res->addHeader("Access-Control-Allow-Origin",  "*");
    res->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    res->addHeader("Cache-Control",                "no-cache");
    req->send(res);
  });

  // OPTIONS /data — CORS pre-flight (some browsers send this before GET)
  server.on("/data", HTTP_OPTIONS, [](AsyncWebServerRequest* req) {
    AsyncWebServerResponse* res = req->beginResponse(204);
    res->addHeader("Access-Control-Allow-Origin",  "*");
    res->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    res->addHeader("Access-Control-Allow-Headers", "Content-Type");
    req->send(res);
  });

  // GET / — mini dashboard: open http://<ip>/ in Safari on your iPad (same WiFi)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send_P(200, "text/html", DASHBOARD_HTML);
  });

  // ── WebSocket ──────────────────────────────────────────────────────────────
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // ── Start HTTP server ──────────────────────────────────────────────────────
  server.begin();
  Serial.println("[HTTP] Server started on port 80");
  Serial.printf("[HTTP] Test in browser: http://%s/\n",
                WiFi.localIP().toString().c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
// loop — read sensors on interval, push to WebSocket clients
// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  ws.cleanupClients();   // free memory from disconnected WS clients

  unsigned long now = millis();
  if (now - lastRead >= READ_INTERVAL_MS) {
    lastRead = now;
    readSensors();
    String payload = buildJson();
    Serial.println("[Data] " + payload);
    ws.textAll(payload);  // push to every connected WebSocket client
  }
}
