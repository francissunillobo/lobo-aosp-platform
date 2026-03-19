# Lobo AOSP Platform — Server & Build Setup

This document describes the complete setup of the Lobo AOSP build environment
on the Hetzner dedicated server.

---

## 1. Overview

The build environment consists of three distinct git repositories plus
AOSP source trees managed by `repo`.

```
/root/lobo-aosp/
├── lobo-aosp-platform/     ← platform repo (this repo, public)
├── lobo-aosp-scripts/      ← server scripts repo (private)
├── raspi5-aosp/            ← RPi5 AOSP tree (repo sync, ~200 GB)
├── vim3-aosp/              ← VIM3 AOSP tree (future)
└── CLAUDE.md               ← Claude Code context file
```

---

## 2. Architecture — Platform vs AOSP Tree

### Key design principle

`lobo-aosp-platform` lives **outside** all AOSP trees. The AOSP trees
contain **symlinks** pointing into it. Git never tracks the symlinks —
they are infrastructure created by the setup script on each server.

```
lobo-aosp-platform/          ← one repo, shared across all hardware targets
├── vendor/lobo/             ← common code (libs, services, apps) — same for all targets
└── projects/
    ├── rpi5_custom/         ← RPi5-specific board config & HAL
    └── vim3_custom/         ← VIM3-specific board config & HAL (future)
```

### Per-AOSP-tree symlinks

When building for **RPi5**:
```
raspi5-aosp/vendor/
├── lobo/              → /root/lobo-aosp/lobo-aosp-platform/vendor/lobo
└── projects/
    └── rpi5_custom/   → /root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom
```

When building for **VIM3** (future):
```
vim3-aosp/vendor/
├── lobo/              → /root/lobo-aosp/lobo-aosp-platform/vendor/lobo  (same)
└── projects/
    └── vim3_custom/   → /root/lobo-aosp/lobo-aosp-platform/projects/vim3_custom
```

### Why this matters

- `raspi5-aosp` never sees `vim3_custom/` — AOSP only scans what is symlinked
- `vim3-aosp` never sees `rpi5_custom/`
- Common code in `vendor/lobo/` is shared — one change applies to all targets
- `lobo-aosp-platform` is never inside `vendor/` of any AOSP tree, so AOSP's
  board scanner (`find -L vendor -maxdepth 4`) never finds duplicate BoardConfig.mk files

---

## 3. Why ALLOW_BP_UNDER_SYMLINKS=true

AOSP's file scanner (Soong, `build/soong/ui/build/finder.go`) does **not**
follow symlinks by default when discovering `AndroidProducts.mk`, `Android.bp`,
and `Android.mk` files. Since `vendor/lobo` and `vendor/projects/rpi5_custom`
are symlinks, the scanner would miss them without this flag.

**Set in:** `vendor/lobo/vendorsetup.sh` (sourced automatically during
`source build/envsetup.sh`).

```bash
export ALLOW_BP_UNDER_SYMLINKS=true
```

---

## 4. Manifest — What Changed and Why

The `manifest_brcm_rpi.xml` in the raspberry-vanilla fork previously
contained this entry:

```xml
<!-- OLD — caused duplicate BoardConfig.mk errors -->
<project name="francissunillobo/lobo-aosp-platform"
         path="vendor/lobo_platform" remote="github" revision="main">
  <linkfile src="vendor/lobo" dest="vendor/lobo" />
  <linkfile src="projects/rpi5_custom" dest="vendor/projects/rpi5_custom" />
</project>
```

**Problem:** Placing the repo at `vendor/lobo_platform` put all its contents
inside `vendor/`. AOSP's `find -L vendor -maxdepth 4` found `BoardConfig.mk`
at two separate paths:
- `vendor/lobo_platform/projects/rpi5_custom/BoardConfig.mk` (direct)
- `vendor/projects/rpi5_custom/BoardConfig.mk` (via linkfile symlink)

**Fix:** The entry was removed from the manifest entirely.
`lobo-aosp-platform` is now a standalone git clone managed independently,
not via `repo sync`. Symlinks are created by `setup-platform-links.sh`.

Current local manifest at
`raspi5-aosp/.repo/local_manifests/manifest_brcm_rpi.xml` no longer
contains the `lobo-aosp-platform` project entry.

> **TODO:** Push this manifest change to the raspberry-vanilla fork on GitHub
> so future `repo sync` runs do not re-add `vendor/lobo_platform`.

---

## 5. Setting Up a New Server from Scratch

### Step 1 — Server setup
```bash
bash /path/to/setup-hetzner-server.sh
```

### Step 2 — Clone lobo-aosp-platform
```bash
cd /root/lobo-aosp
git clone https://github.com/francissunillobo/lobo-aosp-platform.git
```

### Step 3 — Clone the private scripts repo
```bash
git clone git@github.com:francissunillobo/lobo-aosp-scripts.git /root/lobo-aosp/scripts
```

### Step 4 — Initialise and sync AOSP tree (RPi5)
```bash
mkdir -p /root/lobo-aosp/raspi5-aosp
cd /root/lobo-aosp/raspi5-aosp
repo init -u https://github.com/francissunillobo/raspberry-vanilla_android-15.0.0_r14 \
     -m manifest/manifest_brcm_rpi.xml -b main
# copy local manifests (manifest_utilities.xml, remove_projects.xml) as needed
repo sync -c -j16 --no-tags --no-clone-bundle 2>&1 | tee ~/sync.log
```

### Step 5 — Create platform symlinks
```bash
bash /root/lobo-aosp/scripts/setup-platform-links.sh raspi5-aosp rpi5_custom
```

### Step 6 — Build
```bash
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
lunch rpi5_custom-trunk_staging-userdebug
make -j12 2>&1 | tee ~/build.log
```

---

## 6. Day-to-Day Build Commands

```bash
# Attach to tmux
tmux attach -t aosp

# Source + lunch (always run these first)
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
lunch rpi5_custom-trunk_staging-userdebug

# Full build
make -j12 2>&1 | tee ~/build.log

# Build specific modules
make -j12 FanControlService NameService MySystemApp MyUserApp FanSettingsService

# Check errors
grep -E "error:|FAILED" ~/build.log | head -50
```

---

## 7. Repository Index

| Repository | Visibility | Location | Purpose |
|---|---|---|---|
| lobo-aosp-platform | Public | github.com/francissunillobo/lobo-aosp-platform | Common platform code, project configs |
| lobo-aosp-scripts | **Private** | github.com/francissunillobo/lobo-aosp-scripts | Server setup & automation scripts |
| raspberry-vanilla fork | Public | github.com/francissunillobo/raspberry-vanilla_android-15.0.0_r14 | AOSP manifest for RPi5 |

---

## 8. Server Info

| | |
|---|---|
| Provider | Hetzner Dedicated |
| IP | 148.251.152.29 |
| SSH alias | `lobo-hetzner` |
| CPU | Intel Xeon E5-1650 v3 (6 cores / 12 threads) |
| RAM | 64 GB ECC |
| Storage | 2× 2 TB HDD (RAID 1) |
| OS | Ubuntu 22.04 LTS |
| Build jobs | `make -j12` |
