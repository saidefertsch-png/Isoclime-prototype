# IsoClime ESP32 Dashboard

Real-time microclimate sensor dashboard — displays live temperature and humidity data from an ESP32 sensor node.

---

## 🌐 Live site

Default GitHub Pages URL (always works, no setup needed):
**https://saidefertsch-png.github.io/Isoclime-prototype/**

Custom domain (once you've configured it — see below):
**YOUR-DASHBOARD-DOMAIN-HERE**

---

## 🚀 Publishing the site

### Step 1 — Enable GitHub Pages

1. Go to your repo → **Settings** → **Pages** (left sidebar).
2. Under **"Build and deployment"**, set **Source** to **"GitHub Actions"**.
3. Click **Save**.

The deploy workflow (`.github/workflows/deploy.yml`) runs automatically on every merge to `main`. The site is live within ~1 minute.

### Step 2 — Merge this PR

1. Open this pull request on GitHub.
2. Click the green **"Merge pull request"** button, then **"Confirm merge"**.

Done. The default GitHub Pages URL is live immediately. Skip to the hardware setup below unless you also want a custom domain.

---

## 🌍 Custom domain setup (any registrar)

> **This dashboard is intentionally hosted on a separate domain from the main IsoClime site.**
> Use a different domain name — do not reuse the domain for the main site.

### Step 1 — Add DNS records at your registrar

Log in to your domain registrar's DNS panel and add these records for `your-dashboard-domain.com` (substitute your actual domain throughout):

**Four A records — point the root domain (`@`) to GitHub's servers:**

| Type | Host | Value |
|------|------|-------|
| A | `@` | `185.199.108.153` |
| A | `@` | `185.199.109.153` |
| A | `@` | `185.199.110.153` |
| A | `@` | `185.199.111.153` |

**One CNAME record — point `www` to GitHub Pages:**

| Type | Host | Value |
|------|------|-------|
| CNAME | `www` | `saidefertsch-png.github.io` |

These are the same four IP addresses for every GitHub Pages site in the world.

### Step 2 — Wait for DNS propagation

DNS changes take **10–30 minutes** on average (up to 48 hours). Before moving on, verify propagation:

1. Go to **[dnschecker.org](https://dnschecker.org)**.
2. Enter your domain, select record type **A**, click **Search**.
3. When green checkmarks appear showing `185.199.108.153`, DNS has propagated. ✅

> ⏳ Just added records? Wait at least 15–30 minutes before checking.

### Step 3 — Set the custom domain in GitHub Pages settings

> **🚨 Do this step ONLY AFTER dnschecker.org shows green.** Setting the domain before DNS propagates causes "DNS check unsuccessful" — this is not permanent, just premature.

1. Go to your repo → **Settings** → **Pages**.
2. Under **"Custom domain"**, type your domain exactly (e.g. `dashboard.yourdomain.com`) and click **Save**.
3. GitHub runs a DNS check. When it passes (green ✅), tick **"Enforce HTTPS"** and click **Save**.
4. If it still shows "DNS check unsuccessful" after dnschecker.org is green: wait 5 minutes, then click **"Check again"**. Repeat until it passes.

### Step 4 — Update the CNAME file in this repo

> ⚠️ **Required before merging if you have a custom domain.** The `CNAME` file currently contains a placeholder (`YOUR-DASHBOARD-DOMAIN-HERE`). GitHub Pages will fail DNS verification until you replace it with your real domain. If you have **no custom domain** and just want the default GitHub Pages URL, delete the `CNAME` file entirely.

Edit the `CNAME` file at the root of this repo and replace the placeholder with your actual domain:

```
your-dashboard-domain.com
```

Commit and push. GitHub Pages uses this file to serve the right domain.

### 🛠️ Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| "DNS check unsuccessful" right after adding records | DNS hasn't propagated yet | Wait 15–60 min, check dnschecker.org, then click "Check again" |
| "DNS check unsuccessful" but dnschecker.org is green | GitHub cached the old failure | Click **"Check again"** in GitHub Pages Settings |
| DNS shows no records after several hours | Records not saved | Go back to your registrar's DNS panel and confirm all 5 records are there |
| Site loads at default URL but not custom domain | CNAME file not updated | Edit `CNAME` in the repo — replace `YOUR-DASHBOARD-DOMAIN-HERE` with your real domain, then commit and push |
| GitHub Pages shows "Invalid CNAME" or keeps using old domain | CNAME file still has placeholder | Same fix as above |

---

## 🗂️ Project structure

| File | Purpose |
|---|---|
| `index.html` | The entire single-page app — all HTML, CSS, and JS in one file |
| `arduino/isoclime_esp32/isoclime_esp32.ino` | ESP32 Arduino sketch — serves live sensor data to the dashboard |
| `arduino/isoclime_esp32/secrets.h.example` | Template for WiFi credentials (copy to `secrets.h`, never commit) |
| `.github/workflows/deploy.yml` | GitHub Actions workflow that auto-deploys to GitHub Pages |

---

## 🔌 ESP32 Hardware Setup — connecting real sensors to the dashboard

### What you need

| Part | Purpose | Notes |
|---|---|---|
| ESP32 dev board | Microcontroller + WiFi | Any 38-pin ESP32 works (e.g. ESP32-WROOM-32) |
| 3 × DS18B20 sensors | Zone temperatures (z1, z2, z3) | Waterproof stainless-steel probes recommended for outdoor use |
| 1 × DHT22 sensor | Ambient temp + humidity (control sensor) | Comes with or without a breakout board |
| 4.7 kΩ resistor | Pull-up for DS18B20 data line | 1 resistor shared by all 3 sensors |
| Breadboard + jumpers | Wiring | — |

> **Only have one DHT22?**  Open the sketch and change `#define SINGLE_SENSOR_MODE  0` to `1`.
> The three zone temperatures will be generated from the single DHT22 reading with small offsets,
> so the dashboard works immediately while you source additional sensors.

---

### Wiring diagram

```
ESP32 3.3 V ─────┬──────────────────────────────┐
                 │                              │
               [4.7 kΩ]                       [DHT22]
                 │                              │  pin 1 → 3.3 V
ESP32 pin 5 ─────┼── DS18B20 Zone 1 (data)     │  pin 2 → ESP32 pin 4
                 ├── DS18B20 Zone 2 (data)     │  pin 3 → n/c
                 └── DS18B20 Zone 3 (data)     │  pin 4 → GND
                                               │
ESP32 GND ────────── DS18B20 × 3 (GND) ────── ┘  (DHT22 GND)
```

All three DS18B20 sensors share **the same three wires** (pin 5, 3.3 V, GND).
Each sensor has a unique 64-bit address so the ESP32 can tell them apart.
When you first flash the sketch, the Serial Monitor prints each sensor's address next to "Zone 1 / 2 / 3" — that's how you know which physical probe is which zone.

---

### Step 1 — Install libraries in Arduino IDE

Open **Sketch → Library Manager** (or **Sketch → Manage Libraries…**) and install:

| Library | Author | Notes |
|---|---|---|
| **ESP Async WebServer** | ESP32Async | ⚠️ Must be paired with **AsyncTCP by ESP32Async** (see below). Delete any older Me-No-Dev or Khoi Hoang versions of these two libraries first. |
| **AsyncTCP** | ESP32Async | ⚠️ Required companion to ESP Async WebServer above. Without this you get `AsyncTCP.h: No such file or directory`. If you get a `discards qualifiers` error instead, an old Me-No-Dev AsyncTCP is still installed — remove it first. |
| **DHT sensor library** | Adafruit | |
| **Adafruit Unified Sensor** | Adafruit | Required by the DHT library |
| **OneWire** | Jim Studt / Paul Stoffregen | Skip if using SINGLE_SENSOR_MODE |
| **DallasTemperature** | Miles Burton | Skip if using SINGLE_SENSOR_MODE |

### Step 2 — Add the ESP32 board to Arduino IDE (first time only)

1. **File → Preferences**
2. Paste this URL in *Additional Boards Manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. **Tools → Board → Boards Manager** → search **esp32** → click **Install**
4. **Tools → Board → ESP32 Arduino → ESP32 Dev Module**

### Step 3 — Configure the sketch

**WiFi credentials (recommended method):**

Copy `arduino/isoclime_esp32/secrets.h.example` → `arduino/isoclime_esp32/secrets.h` and fill in your network names and passwords. The `secrets.h` file is listed in `.gitignore` so it will never be accidentally committed to GitHub.

The example file has two blocks — home and school. Fill in both once, then just flip the toggle in the main sketch:

```cpp
#define SCHOOL_WIFI  0   // 0 = home (DHCP),  1 = school (static IP 11.0.8.106)
```

When `SCHOOL_WIFI 1` is set, the ESP32 uses the fixed IP **11.0.8.106** — the dashboard URL at school is always `http://11.0.8.106/`, no need to look it up.

If you only have one DHT22 for now, also change:

```cpp
#define SINGLE_SENSOR_MODE  1   // was 0
```

### Step 4 — Flash the ESP32

1. Plug the ESP32 into your computer via USB.
2. **Tools → Port** → select the COM / `/dev/tty…` port that appeared.
3. Click **Upload** (the → arrow button).
4. Once uploaded, open **Tools → Serial Monitor** at **115 200 baud**.
5. You will see the IP address printed:
   - **At home:** dynamic address, e.g. `[WiFi] Connected!  IP address: 192.168.1.42`
   - **At school:** always `[WiFi] Connected!  IP address: 11.0.8.106`

---

## 📱 Viewing the dashboard on an iPad (or any device)

The IsoClime dashboard is a normal web page. Any device with a browser that is on the **same WiFi network as the ESP32** can use it.

### Option A — Use the hosted GitHub Pages version (easiest)

1. On your iPad, open **Safari** (or Chrome).
2. Go to your dashboard URL — either your custom domain (e.g. `https://dashboard.yourdomain.com`) or the default GitHub Pages URL `https://saidefertsch-png.github.io/Isoclime-prototype/`.
3. The dashboard loads. In the **ESP32 Connection** bar at the top, type the IP address you noted in Step 4 (e.g. `192.168.1.42`).
4. Choose transport: **HTTP** (simpler) or **WebSocket** (lower latency).
5. Tap **Connect**.
6. The status badge turns **LIVE** and all sensor values update in real time.

> ⚠️ **Mixed-content note (HTTP vs HTTPS):** The GitHub Pages site is served over HTTPS.
> Browsers block *HTTP* fetch requests from an HTTPS page (mixed content).
> If the connection fails with an error like "blocked:mixed-content", use the **WebSocket** transport instead — or use Option B below.

### Option B — Open the file locally on the iPad (no mixed-content issue)

1. Download `index.html` from this repo (or AirDrop it from your computer).
2. Open it with **Safari** directly from the Files app.
3. Enter the ESP32 IP and tap **Connect** as above.
   Because the page is served from `file://`, the browser allows plain HTTP fetch to local devices.

### Option C — Serve the dashboard from the ESP32 itself

If you want the ESP32 to host the page (no internet needed):
1. Copy the contents of `index.html` into a C++ string in the sketch and add a new route:
   ```cpp
   server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
     req->send_P(200, "text/html", INDEX_HTML);
   });
   ```
2. The iPad opens `http://192.168.1.42/` and the dashboard connects to the same device automatically.

### Making the dashboard a Home Screen app on iPad

1. Open the dashboard in Safari.
2. Tap the **Share** button (box with arrow).
3. Tap **Add to Home Screen**.
4. Give it a name (e.g. "IsoClime") and tap **Add**.
5. It now behaves like a native app — full-screen, no browser bar.