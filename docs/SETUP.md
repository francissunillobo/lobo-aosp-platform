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
contain **bind-mounted directories** pointing into it. Git never tracks
these mount points — they are infrastructure set up on each server.

```
lobo-aosp-platform/          ← one repo, shared across all hardware targets
├── vendor/lobo/             ← common code (libs, services, apps) — same for all targets
└── projects/
    ├── rpi5_custom/         ← RPi5-specific board config & HAL
    └── vim3_custom/         ← VIM3-specific board config & HAL (future)
```

### Per-AOSP-tree bind mounts

When building for **RPi5**:
```
raspi5-aosp/vendor/
├── lobo/              ← bind mount → lobo-aosp-platform/vendor/lobo/
└── projects/
    └── rpi5_custom/   ← bind mount → lobo-aosp-platform/projects/rpi5_custom/
```

When building for **VIM3** (future):
```
vim3-aosp/vendor/
├── lobo/              ← bind mount → lobo-aosp-platform/vendor/lobo/  (same)
└── projects/
    └── vim3_custom/   ← bind mount → lobo-aosp-platform/projects/vim3_custom/
```

### Why this matters

- `raspi5-aosp` never sees `vim3_custom/` — AOSP only scans what is bind-mounted
- `vim3-aosp` never sees `rpi5_custom/`
- Common code in `vendor/lobo/` is shared — one change applies to all targets
- `lobo-aosp-platform` is never inside `vendor/` of any AOSP tree, so AOSP's
  board scanner (`find -L vendor -maxdepth 4`) never finds duplicate `BoardConfig.mk`

---

## 3. Why bind mounts (not symlinks)

Symlinks were tried first and caused two critical failures:

**Problem 1 — Soong scanner hung (50+ GB RAM)**
`ALLOW_BP_UNDER_SYMLINKS=true` caused Soong to follow ALL symlinks in the
tree including `.repo/` git-internal symlinks, scanning gigabytes of git
objects indefinitely.

**Problem 2 — Duplicate BoardConfig.mk**
AOSP's `find -L vendor -maxdepth 4` found `BoardConfig.mk` at two paths
simultaneously and crashed with a duplicate product error.

**Why bind mounts solve both:**
Soong sees real directories — no special flags needed.
`find` sees real directories — no duplication.
Deleting `raspi5-aosp/` only removes empty mount point directories —
files in `lobo-aosp-platform/` are never affected.

---

## 4. Setting Up a New Server from Scratch

### Step 1 — Clone lobo-aosp-platform
```bash
cd /root/lobo-aosp
git clone https://github.com/francissunillobo/lobo-aosp-platform.git
```

### Step 2 — Clone the private scripts repo
```bash
git clone git@github.com:francissunillobo/lobo-aosp-scripts.git /root/lobo-aosp/scripts
```

### Step 3 — Set build mode
```bash
~/scripts/build-mode.sh
```

### Step 4 — Initialise and sync AOSP tree (RPi5, Android 16)
```bash
mkdir -p /root/lobo-aosp/raspi5-aosp
cd /root/lobo-aosp/raspi5-aosp

repo init -u https://android.googlesource.com/platform/manifest \
          -b android-16.0.0_r3 --depth=1

curl -o .repo/local_manifests/manifest_brcm_rpi.xml -L \
  https://raw.githubusercontent.com/raspberry-vanilla/android_local_manifest/android-16.0.0_r3/manifest_brcm_rpi.xml \
  --create-dirs

curl -o .repo/local_manifests/remove_projects.xml -L \
  https://raw.githubusercontent.com/raspberry-vanilla/android_local_manifest/android-16.0.0_r3/remove_projects.xml

repo sync -c -j16 --no-tags --no-clone-bundle 2>&1 | tee ~/sync-android16.log
```

### Step 5 — Create bind mounts
```bash
mkdir -p /root/lobo-aosp/raspi5-aosp/vendor/lobo
mkdir -p /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom

sudo mount --bind \
    /root/lobo-aosp/lobo-aosp-platform/vendor/lobo \
    /root/lobo-aosp/raspi5-aosp/vendor/lobo

sudo mount --bind \
    /root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom \
    /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom
```

Add to `/etc/fstab` to persist across reboots:
```
/root/lobo-aosp/lobo-aosp-platform/vendor/lobo  /root/lobo-aosp/raspi5-aosp/vendor/lobo  none  bind  0  0
/root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom  /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom  none  bind  0  0
```

### Step 6 — Build
```bash
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
lunch rpi5_custom-trunk_staging-userdebug
make bootimage systemimage vendorimage -j$(nproc) 2>&1 | tee ~/build-android16.log
```

### Step 7 — Create flashable image
```bash
./rpi5-mkimg.sh
```

---

## 5. Day-to-Day Build Commands

```bash
# Attach to tmux
tmux attach -t aosp

# Source + lunch (always run these first)
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
lunch rpi5_custom-trunk_staging-userdebug

# Build the 3 images needed for RPi5
make bootimage systemimage vendorimage -j$(nproc) 2>&1 | tee ~/build-android16.log

# Build specific modules only
make FanControlService NameService MySystemApp MyUserApp FanSettingsService -j$(nproc)

# Check errors
grep -E "error:|FAILED" ~/build-android16.log | head -50
```

---

## 6. Repository Index

| Repository | Visibility | Location | Purpose |
|---|---|---|---|
| lobo-aosp-platform | Public | github.com/francissunillobo/lobo-aosp-platform | Common platform code, project configs |
| lobo-aosp-scripts | **Private** | github.com/francissunillobo/lobo-aosp-scripts | Server setup & automation scripts |
| raspberry-vanilla manifest | Public | github.com/raspberry-vanilla/android_local_manifest | RPi5 AOSP manifest (android-16.0.0_r3) |

---

## 7. Server Info

| | |
|---|---|
| Provider | Hetzner Dedicated |
| IP | See private server notes |
| SSH alias | `lobo-hetzner` |
| CPU | Intel Xeon E5-1650 v3 (6 cores / 12 threads) |
| RAM | 64 GB ECC |
| Storage | 2× 2 TB HDD (RAID 1) |
| OS | Ubuntu 22.04 LTS |
| Build jobs | `make -j$(nproc)` |
