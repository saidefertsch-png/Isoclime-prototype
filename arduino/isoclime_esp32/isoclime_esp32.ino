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

// WiFi credentials — two options:
//
//  Option 1 (recommended): create a file called  secrets.h  in this folder
//  with the two lines below and it will be included automatically.
//  secrets.h is listed in .gitignore so it will NOT be committed to GitHub.
//
//  Option 2 (quick start): replace the placeholder strings below directly.
//  Do NOT commit real credentials to a public repository.
//
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  const char* WIFI_SSID = "YOUR_WIFI_SSID";    // ← replace with your network name
  const char* WIFI_PASS = "YOUR_WIFI_PASSWORD"; // ← replace with your password
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

  // GET / — simple info page you can open in any browser to confirm the node is up
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    String ip = WiFi.localIP().toString();
    String html =
      "<!doctype html><html><head>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>IsoClime Node</title>"
      "<style>body{font-family:sans-serif;max-width:480px;margin:40px auto;padding:0 16px;}"
      "pre{background:#f0fdf4;border:1px solid #a7f3d0;border-radius:8px;padding:12px;}"
      "a{color:#059669;}</style></head><body>"
      "<h2>&#127807; IsoClime ESP32 Node</h2>"
      "<p>Node is <strong>online</strong>.</p>"
      "<p><a href='/data'>GET /data</a> &mdash; live JSON reading</p>"
      "<p>WebSocket: <code>ws://" + ip + "/ws</code></p>"
      "<pre id='d'>Loading…</pre>"
      "<script>fetch('/data').then(r=>r.json()).then(d=>{"
      "document.getElementById('d').textContent=JSON.stringify(d,null,2)});</script>"
      "</body></html>";
    req->send(200, "text/html", html);
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
