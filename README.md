# IsoClime Prototype

AI-powered microclimate analysis for your neighborhood — detect heat stress, humidity risk, and get actionable solutions.

---

## 🌐 Live site

Once the PR is merged and GitHub Pages is enabled, the site is live at:

**https://saidefertsch-png.github.io/Isoclime-prototype/**

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

## 🌍 Custom domain setup (fix `InvalidDNSError`)

GitHub Pages is configured to serve the site at **https://isoclimehn.com** via the `CNAME` file.
To resolve the `InvalidDNSError` ("Domain's DNS record could not be retrieved"), you must add DNS records at your domain registrar (Namecheap, GoDaddy, Cloudflare, etc.).

### Step 1 — Add A records for the apex domain (`isoclimehn.com`)

Log in to your registrar's DNS settings and add **four A records**:

| Type | Name/Host | Value | TTL |
|------|-----------|-------|-----|
| A | `@` | `185.199.108.153` | 3600 |
| A | `@` | `185.199.109.153` | 3600 |
| A | `@` | `185.199.110.153` | 3600 |
| A | `@` | `185.199.111.153` | 3600 |

### Step 2 — Add a CNAME record for `www`

| Type | Name/Host | Value | TTL |
|------|-----------|-------|-----|
| CNAME | `www` | `saidefertsch-png.github.io` | 3600 |

### Step 3 — Set the custom domain in GitHub Pages settings

1. Go to your repo → **Settings** → **Pages**.
2. Under **"Custom domain"**, enter `isoclimehn.com` and click **Save**.
3. Once the DNS check passes, tick **"Enforce HTTPS"**.

### Step 4 — Wait for DNS propagation

DNS changes can take **10 minutes to 48 hours** to fully propagate. After that, visiting **https://isoclimehn.com** will load the site.

> **Note:** The `CNAME` file at the root of this repo already contains `IsoclimeHN.com`. DNS is case-insensitive, so `isoclimehn.com`, `IsoclimeHN.com`, and `ISOCLIMEHN.COM` all resolve to the same domain. No repository changes are needed — only the registrar-level DNS records above.

---

## 🗂️ Project structure

| File | Purpose |
|---|---|
| `index.html` | The entire single-page app — all HTML, CSS, and JS in one file |
| `.github/workflows/deploy.yml` | GitHub Actions workflow that auto-deploys to GitHub Pages |