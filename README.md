# IsoClime Prototype

AI-powered microclimate analysis for your neighborhood — detect heat stress, humidity risk, and get actionable solutions.

---

## 🌐 Live site

Once the PR is merged and GitHub Pages is enabled, the site is live at:

**https://isoclimehn.shop** (custom domain — see setup instructions below)

or at the default GitHub Pages URL: **https://saidefertsch-png.github.io/Isoclime-prototype/**

---

## 🚀 How to merge the PR and publish the site

### Step 1 — Mark the PR as ready

The PR starts in **Draft** mode. To merge it:

1. Open **[Pull Request #1](https://github.com/saidefertsch-png/Isoclime-prototype/pull/1)** on GitHub.
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

> ### 🟢 DNS has propagated — here is the one remaining step
>
> If you can see **all green checkmarks** for `isoclimehn.shop` (type A) on [dnschecker.org](https://dnschecker.org) but GitHub Pages still shows **"DNS check unsuccessful"**, GitHub just needs to recheck. It cached the failure from before propagation completed.
>
> **Do this right now:**
> 1. Go to your repo → **Settings** → **Pages**
> 2. Click the **"Check again"** button next to the red DNS error
> 3. Wait 10–15 seconds — the red error should turn into a green ✅
> 4. Once green, tick **"Enforce HTTPS"**
>
> That is the only step remaining. You do not need to change any DNS records or re-enter the domain.

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
| `.github/workflows/deploy.yml` | GitHub Actions workflow that auto-deploys to GitHub Pages |