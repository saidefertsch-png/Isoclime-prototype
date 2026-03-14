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

The `CNAME` file in this repo is already set to `isoclimehn.shop`.
Follow these steps to finish connecting your Namecheap domain to GitHub Pages.

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

### Step 2 — Set the custom domain in GitHub Pages settings

1. Go to your repo → **Settings** → **Pages**.
2. Under **"Custom domain"**, type `isoclimehn.shop` and click **Save**.
3. GitHub will run a DNS check. Once it passes (green checkmark ✅), tick **"Enforce HTTPS"**.

### Step 3 — Wait for DNS propagation

DNS changes on Namecheap typically take **10–30 minutes**, but can take up to 48 hours. Once propagated, your site will be live at **https://isoclimehn.shop**.

> **Tip:** You can check whether DNS has propagated by visiting [dnschecker.org](https://dnschecker.org) and searching for `isoclimehn.shop` with record type **A**. When you see `185.199.108.153` (or any of the four IPs above) appearing globally, you're good to go.

---

## 🗂️ Project structure

| File | Purpose |
|---|---|
| `index.html` | The entire single-page app — all HTML, CSS, and JS in one file |
| `.github/workflows/deploy.yml` | GitHub Actions workflow that auto-deploys to GitHub Pages |