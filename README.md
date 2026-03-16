# IsoClime Prototype

AI-powered microclimate analysis for your neighborhood — detect heat stress, humidity risk, and get actionable solutions.

---

## 🔴 Are you seeing an old dark-theme version of the site?

The site redirects correctly to `isoclimehn.shop`, but you might be seeing the **old dark/cyberpunk version** instead of the new light teal version. Here is why and how to fix it in 3 clicks:

**Why it happens:** GitHub Pages only deployed once — when the first basic PR was merged. All the big improvements (light teal theme, Partner page, 100+ neighborhoods, IsoClime logo, inquiry form) are on *this* PR and haven't been deployed yet because **this PR is still in Draft**.

**Fix (3 clicks):**
1. Open **[Pull Request #3](https://github.com/saidefertsch-png/Isoclime-prototype/pull/3)** — that's this PR.
2. Click **"Ready for review"** (bottom of the PR page) — this takes it out of Draft mode.
3. Click the green **"Merge pull request"** button, then **"Confirm merge"**.

The deploy workflow runs automatically after the merge. Within ~60 seconds the new version is live. If the site still looks old after 60 seconds, press **Ctrl+Shift+R** (Windows/Linux) or **⌘+Shift+R** (Mac) to hard-refresh your browser.

---

## 🌐 Live site

**https://isoclimehn.shop** (custom domain)

or at the default GitHub Pages URL: **https://saidefertsch-png.github.io/Isoclime-prototype/**

---

## 🚀 How to merge this PR and publish the updated site

The PR starts in **Draft** mode. To merge it:

1. Open **[Pull Request #3](https://github.com/saidefertsch-png/Isoclime-prototype/pull/3)** on GitHub (that's this PR).
2. Scroll to the bottom and click **"Ready for review"**.

### Step 2 — Merge the PR

1. Click the green **"Merge pull request"** button.
2. Click **"Confirm merge"**.

### Step 3 — Enable GitHub Pages (one-time setup)

1. Go to your repo → **Settings** → **Pages** (left sidebar).
2. Under **"Build and deployment"**, set **Source** to **"GitHub Actions"**.
3. Click **Save**.

The deploy workflow (`.github/workflows/deploy.yml`) will run automatically on every merge to `main`, and the site will be live within ~1 minute at the URL above.

---

## 🌍 Custom domain setup — `isoclimehn.shop` on Namecheap

> ### 🟢 The site is already live at `http://isoclimehn.shop/` — you only need to enable HTTPS
>
> The deployment has succeeded. The URL `http://isoclimehn.shop/` (HTTP) is **already working**.
> The only reason it may feel "broken" is that HTTPS is not yet enabled — and HTTPS requires GitHub's one-time DNS verification check to pass.
>
> **Three-step checklist to finish:**
>
> **Step A — Open your browser and confirm HTTP loads the site:**
> Go to `http://isoclimehn.shop/` (note: `http://` not `https://`). If the IsoClime page loads, DNS is fine and you are 90% done. Skip to Step C.
>
> **Step B — If `http://isoclimehn.shop/` does NOT load yet:**
> The DNS record may not have propagated to your network yet, even if dnschecker.org shows green. Wait 15–30 more minutes and try again. You can also try on your mobile data connection (bypasses home router cache).
>
> **Step C — Enable HTTPS (the final step):**
> 1. Go to your repo → **Settings** → **Pages**
> 2. Under "Custom domain", the field should already show `isoclimehn.shop`
> 3. If there is a red error ("DNS check unsuccessful"): click the **"Check again"** button. Wait 10–15 seconds. Repeat until it turns green ✅
> 4. Once the check is green, tick **"Enforce HTTPS"** and click **Save**
> 5. Done — `https://isoclimehn.shop/` is now live
>
> **If the custom domain field is blank or missing:** type `isoclimehn.shop` in the field and click Save, then follow Step C above.
>
> **If you want to force a fresh deployment at any time:** go to the **Actions** tab → select **"Deploy to GitHub Pages"** → click **"Run workflow"** → click the green **"Run workflow"** button.

The `CNAME` file in this repo is already set to `isoclimehn.shop`.
Follow these one-time setup steps below if you ever need to redo the configuration from scratch.

### ⚠️ Step 0 — Switch DNS type to "Namecheap BasicDNS" (required first!)

If you open the **Advanced DNS** tab and the **HOST RECORDS** section shows no "Add New Record" button — only a message about changing the DNS type — it means your domain is currently on **Namecheap Web Hosting DNS**, which does not let you edit host records.

You must switch to **BasicDNS** first:

1. Log in to [namecheap.com](https://www.namecheap.com) → **Domain List** → click **Manage** next to `isoclimehn.shop`.
2. Click the **Domain** tab (the house icon).
3. Find the **Nameservers** section and open the dropdown. It likely says "Namecheap Web Hosting DNS".
4. Change it to **"Namecheap BasicDNS"**.
5. Click the **✔** (green checkmark) to save.
6. Now click the **Advanced DNS** tab — the **HOST RECORDS** section will show the "Add New Record" button.

### Step 1 — Add DNS records in Namecheap

1. Still on the **Advanced DNS** tab for `isoclimehn.shop`.
2. Delete any existing A records or CNAME records for `@` and `www` that Namecheap may have pre-filled.
3. Add the following records (click **Add New Record** for each one):

**Four A records (apex domain `@`):**

| Type | Host | Value | TTL |
|------|------|-------|-----|
| A Record | `@` | `185.199.108.153` | Automatic |
| A Record | `@` | `185.199.109.153` | Automatic |
| A Record | `@` | `185.199.110.153` | Automatic |
| A Record | `@` | `185.199.111.153` | Automatic |

**One CNAME record (`www` subdomain):**

| Type | Host | Value | TTL |
|------|------|-------|-----|
| CNAME Record | `www` | `saidefertsch-png.github.io` | Automatic |

4. Click the **✔ (Save All Changes)** button.

### Step 2 — Wait for DNS propagation and verify it worked

After saving DNS records in Namecheap, the changes must spread to DNS servers worldwide before GitHub can see them. **This takes 10–30 minutes on average, but can take up to 48 hours.** Do not skip this step — setting the GitHub Pages custom domain before DNS has propagated will always give you "DNS check unsuccessful".

**How to check (do this before moving to Step 3):**

1. Go to **[dnschecker.org](https://dnschecker.org)**.
2. Type `isoclimehn.shop` in the search box and select record type **A**.
3. Click **Search**.
4. When you see green checkmarks showing `185.199.108.153` (or any of the four GitHub IPs) across most locations, DNS has propagated. ✅

> ⏳ **Just added your records?** Wait at least 15–30 minutes before checking. If dnschecker.org still shows nothing or shows old IPs, just wait a bit longer and refresh.

### Step 3 — Set the custom domain in GitHub Pages settings

> **🚨 Only do this step AFTER dnschecker.org shows green checkmarks for `isoclimehn.shop`.**
> If you set the custom domain before DNS has propagated, GitHub will immediately show "DNS check unsuccessful / InvalidDNSError". That is not a permanent error — it just means GitHub checked too early. Click **"Check again"** once dnschecker.org confirms propagation.

> **🚨 Common mistake — type `.shop`, NOT `.com`!**
> The domain you own is **`isoclimehn.shop`**. Typing `isoclimehn.com` will also give "DNS check unsuccessful" because `.com` has no records pointing to GitHub.

1. Go to your repo → **Settings** → **Pages**.
2. Under **"Custom domain"**, clear the field completely and type exactly:
   ```
   isoclimehn.shop
   ```
   (ends in `.shop` — not `.com`, not `.net`, not anything else)
3. Click **Save**.
4. GitHub will run a DNS check. If it passes (green checkmark ✅), tick **"Enforce HTTPS"** and you're done!
5. If it still shows "DNS check unsuccessful" after dnschecker.org is green: wait 5 more minutes, then click **"Check again"** inside the GitHub Pages settings. Repeat until it passes.

---

### 🛠️ Troubleshooting — "DNS check unsuccessful" even with the correct domain

| Symptom | Cause | Fix |
|---------|-------|-----|
| `isoclimehn.com` in the field | Wrong domain (`.com` ≠ `.shop`) | Clear the field, type `isoclimehn.shop`, click Save |
| `isoclimehn.shop` in the field, just added DNS records | DNS hasn't propagated yet | Wait 15–60 min, check dnschecker.org, then click "Check again" |
| `isoclimehn.shop` in the field, dnschecker.org is green | GitHub cached the old failure | Click **"Check again"** in GitHub Pages Settings |
| dnschecker.org still shows no records after several hours | Records not saved in Namecheap | Go back to Namecheap Advanced DNS and confirm all 5 records are there and saved |

### ❓ Does the `hn` in `isoclimehn.shop` mean it needs Honduras-specific IPs?

**No.** The `hn` is just part of the brand name **IsoClime HN** — it is not the domain extension. The actual top-level domain (TLD) is `.shop`, which is a generic global domain. GitHub Pages uses the same four IP addresses (`185.199.108–111.153`) for **every** custom domain in the world, regardless of what country abbreviation appears in the name. Those IPs point to GitHub's global CDN, which serves visitors in Honduras and everywhere else. No special regional IPs are needed.

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
| **ESP Async WebServer** | Me-No-Dev | Installs AsyncTCP automatically |
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

**WiFi credentials (choose one method):**

- **Recommended:** Copy `arduino/isoclime_esp32/secrets.h.example` to `arduino/isoclime_esp32/secrets.h` and fill in your network name and password. The `secrets.h` file is listed in `.gitignore` so it will never be accidentally committed to GitHub.
  ```cpp
  // secrets.h
  const char* WIFI_SSID = "MyHomeNetwork";
  const char* WIFI_PASS = "MyPassword123";
  ```

- **Quick start:** Open `isoclime_esp32.ino` and edit the two placeholder lines directly — but do **not** commit real credentials to a public repo.

If you only have one DHT22 for now, also change:

```cpp
#define SINGLE_SENSOR_MODE  1   // was 0
```

### Step 4 — Flash the ESP32

1. Plug the ESP32 into your computer via USB.
2. **Tools → Port** → select the COM / `/dev/tty…` port that appeared.
3. Click **Upload** (the → arrow button).
4. Once uploaded, open **Tools → Serial Monitor** at **115 200 baud**.
5. You will see the ESP32's IP address printed, e.g.:
   ```
   [WiFi] Connected!  IP address: 192.168.1.42
   ```
   **Write this IP down** — you will need it in the dashboard.

---

## 📱 Viewing the dashboard on an iPad (or any device)

The IsoClime dashboard is a normal web page. Any device with a browser that is on the **same WiFi network as the ESP32** can use it.

### Option A — Use the hosted GitHub Pages version (easiest)

1. On your iPad, open **Safari** (or Chrome).
2. Go to **https://isoclimehn.shop** (or the GitHub Pages URL).
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