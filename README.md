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

## 🗂️ Project structure

| File | Purpose |
|---|---|
| `index.html` | The entire single-page app — all HTML, CSS, and JS in one file |
| `.github/workflows/deploy.yml` | GitHub Actions workflow that auto-deploys to GitHub Pages |