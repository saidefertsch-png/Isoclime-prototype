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
 *   • ESP Async WebServer  by Me-No-Dev   (also installs AsyncTCP)
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
//  SCHOOL_WIFI  1 = use SCHOOL WiFi credentials
//
//  ★ HOW TO SWITCH WHEN YOU GET TO SCHOOL:
//    Step 1 — Change  #define SCHOOL_WIFI  0  →  #define SCHOOL_WIFI  1  (below)
//    Step 2 — Fill in your school's WiFi name + password in the SCHOOL section.
//    Step 3 — Plug the ESP32 into a computer and click Upload in Arduino IDE.
//    Step 4 — Open Serial Monitor (115200 baud) to see the new IP address.
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

// ── Mini dashboard HTML ───────────────────────────────────────────────────────
// Served at http://<ip>/ — open this URL in Safari on your iPad (or any browser)
// while on the same WiFi as the ESP32. Shows live sensor data without any
// mixed-content restrictions (everything is plain HTTP on the local network).
static const char DASHBOARD_HTML[] PROGMEM = R"html(<!doctype html><html><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>IsoClime Live</title><style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:linear-gradient(135deg,#ecfdf5,#f0fdfa);min-height:100vh;padding:20px;color:#0f2f28}
h1{text-align:center;font-size:22px;font-weight:700;color:#059669;padding:20px 0 6px}
.sub{text-align:center;font-size:13px;color:#0d9488;margin-bottom:24px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:12px;max-width:580px;margin:0 auto}
.card{background:rgba(255,255,255,.8);border:1px solid rgba(52,211,153,.35);border-radius:16px;padding:18px 14px;text-align:center}
.lbl{font-size:10px;color:#0d9488;font-weight:700;text-transform:uppercase;letter-spacing:.6px;margin-bottom:8px}
.val{font-size:32px;font-weight:700;color:#059669}.unit{font-size:13px;color:#0d7377;margin-top:3px}
.st{text-align:center;margin-top:22px;font-size:12px;color:#0d9488}
.dot{display:inline-block;width:7px;height:7px;border-radius:50%;background:#10b981;margin-right:5px;animation:p 1.5s infinite}
@keyframes p{0%,100%{opacity:1}50%{opacity:.3}}
</style></head><body>
<h1>&#127807; IsoClime</h1>
<p class="sub">Live microclimate sensor data &mdash; updates every 2 s</p>
<div class="grid">
<div class="card"><div class="lbl">Zone 1</div><div class="val" id="z1">&mdash;</div><div class="unit">&deg;C</div></div>
<div class="card"><div class="lbl">Zone 2</div><div class="val" id="z2">&mdash;</div><div class="unit">&deg;C</div></div>
<div class="card"><div class="lbl">Zone 3</div><div class="val" id="z3">&mdash;</div><div class="unit">&deg;C</div></div>
<div class="card"><div class="lbl">Control Temp</div><div class="val" id="ct">&mdash;</div><div class="unit">&deg;C</div></div>
<div class="card"><div class="lbl">Humidity</div><div class="val" id="ch">&mdash;</div><div class="unit">%</div></div>
</div>
<p class="st" id="st"><span class="dot"></span>Connecting&hellip;</p>
<script>
function upd(){
  fetch('/data').then(function(r){return r.json();}).then(function(d){
    document.getElementById('z1').textContent=d.z1.toFixed(1);
    document.getElementById('z2').textContent=d.z2.toFixed(1);
    document.getElementById('z3').textContent=d.z3.toFixed(1);
    document.getElementById('ct').textContent=d.ctrl_temp.toFixed(1);
    document.getElementById('ch').textContent=Math.round(d.ctrl_hum);
    document.getElementById('st').innerHTML='<span class="dot"></span>Live &mdash; '+new Date().toLocaleTimeString();
  }).catch(function(){
    document.getElementById('st').innerHTML='<span style="color:#dc2626">&#9888; Connection lost &mdash; retrying&hellip;</span>';
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
// ─────────────────────────────────────────────────────────────────────────────
String buildJson() {
  // Replace NaN with 0.0 so the browser always receives a valid number
  auto safe = [](float v) -> float { return isnan(v) ? 0.0f : v; };
  char buf[128];
  snprintf(buf, sizeof(buf),
    "{\"z1\":%.1f,\"z2\":%.1f,\"z3\":%.1f,\"ctrl_temp\":%.1f,\"ctrl_hum\":%.0f}",
    safe(z1), safe(z2), safe(z3), safe(ctrlTemp), safe(ctrlHum));
  return String(buf);
}

// ─────────────────────────────────────────────────────────────────────────────
// Read all sensors and store in the global floats
// ─────────────────────────────────────────────────────────────────────────────
void readSensors() {
  // Control sensor (DHT22) ─────────────────────────────────────────────────
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) ctrlTemp = t;
  if (!isnan(h)) ctrlHum  = h;

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
  if (r0 > -100) z1 = r0;
  if (r1 > -100) z2 = r1;
  if (r2 > -100) z3 = r2;
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
  Serial.begin(115200);
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
