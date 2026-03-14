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
| `.github/workflows/deploy.yml` | GitHub Actions workflow that auto-deploys to GitHub Pages |