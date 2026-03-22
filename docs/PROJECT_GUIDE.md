# Lobo AOSP Platform — Complete Project Guide

**Author:** Francis Lobo
**Last updated:** 2026-03-21
**Target hardware:** Raspberry Pi 5
**Android version:** Android 16 (android-16.0.0_r3)

**What / Why / How:** This guide explains *what* the platform contains, *why* bind mounts and `projects/` exist, and *how* to build and extend it. For the doc-writing convention used here, see `DOCUMENTATION_STYLE.md`.

---

## Table of Contents

1. [What is this project?](#1-what-is-this-project)
2. [Server infrastructure](#2-server-infrastructure)
3. [Directory structure on server](#3-directory-structure-on-server)
4. [Repository overview](#4-repository-overview)
5. [lobo-aosp-platform — detailed structure](#5-lobo-aosp-platform--detailed-structure)
6. [How lobo-aosp-platform connects to AOSP](#6-how-lobo-aosp-platform-connects-to-aosp)
7. [Bind mounts — what, why and how](#7-bind-mounts--what-why-and-how)
8. [Product inheritance chain](#8-product-inheritance-chain)
9. [Build system files explained](#9-build-system-files-explained)
10. [Setting up from scratch](#10-setting-up-from-scratch)
11. [Day-to-day build commands](#11-day-to-day-build-commands)
12. [Flashing to Raspberry Pi 5](#12-flashing-to-raspberry-pi-5)
13. [Adding a new hardware target](#13-adding-a-new-hardware-target)
14. [Known issues and lessons learned](#14-known-issues-and-lessons-learned)

---

## 1. What is this project?

**Lobo AOSP Platform** is a custom Android OS build for the Raspberry Pi 5.

It is built on top of:
- **AOSP Android 16** — Google's open source Android
- **raspberry-vanilla** — a community project that ports AOSP to Raspberry Pi hardware

On top of these, Lobo adds:
- Custom system services (Fan Control, Name Service)
- Custom apps (MySystemApp, MyUserApp)
- RPi5-specific hardware abstraction layers (HAL) for fan and thermal control
- A clean, multi-target architecture so the same code can run on future hardware (VIM3, etc.)

---

## 2. Server infrastructure

All AOSP builds happen on a dedicated Hetzner server. Building Android locally is not
practical — the full build takes 3-6 hours and requires ~200 GB of disk space.

| Property | Value |
|---|---|
| Provider | Hetzner Dedicated (Server Auction) |
| IP | See private server notes |
| SSH alias | `lobo-hetzner` |
| CPU | Intel Xeon E5-1650 v3 (6 cores / 12 threads @ 3.5 GHz) |
| RAM | 64 GB ECC |
| Swap | 32 GB |
| Storage | 2× 2 TB HDD (RAID 1 — mirrored) |
| OS | Ubuntu 22.04 LTS |
| Build jobs | `make -j12` or `make -j$(nproc)` |

**Why a dedicated server?**
AOSP source is ~100 GB. A full build generates another ~100 GB of output files.
The compiler spawns 12 parallel jobs for hours. A laptop cannot handle this.

**Connect:**
```bash
ssh lobo-hetzner
tmux attach -t aosp       # attach to persistent terminal session
```

**Before building — set server to build mode:**
```bash
~/scripts/build-mode.sh
```
This throttles RAID resync, sets CPU to performance governor, and reduces swap usage.
Run `~/scripts/normal-mode.sh` after the build completes.

---

## 3. Directory structure on server

```
/root/lobo-aosp/
├── lobo-aosp-platform/        ← YOUR CODE — git clone (this repo)
├── raspi5-aosp/               ← AOSP source tree for RPi5 (repo sync, ~200 GB)
│   ├── .repo/                 ← repo tool metadata
│   │   └── local_manifests/   ← custom manifest overlays
│   ├── vendor/
│   │   ├── lobo/              ← BIND MOUNT → lobo-aosp-platform/vendor/lobo/
│   │   └── projects/
│   │       └── rpi5_custom/   ← BIND MOUNT → lobo-aosp-platform/projects/rpi5_custom/
│   └── out/                   ← build output (generated, gitignored)
│       └── target/product/rpi5/
│           ├── boot.img
│           ├── system.img
│           ├── vendor.img
│           └── RaspberryVanillaAOSP16-*.img   ← final flashable image
└── vim3-aosp/                 ← future VIM3 AOSP tree
```

**Key point:** `lobo-aosp-platform` is NOT inside `raspi5-aosp`. It sits alongside it.
The AOSP tree connects to it via bind mounts (explained in section 7).

---

## 4. Repository overview

| Repository | Visibility | URL | Purpose |
|---|---|---|---|
| lobo-aosp-platform | Public | github.com/francissunillobo/lobo-aosp-platform | All custom platform code |
| lobo-aosp-scripts | Private | github.com/francissunillobo/lobo-aosp-scripts | Server setup & build scripts |
| raspberry-vanilla manifest | Public | github.com/raspberry-vanilla/android_local_manifest | RPi5 AOSP manifest overlays |

**AOSP source** is not a git repo — it is managed by Google's `repo` tool which
syncs ~991 separate git repositories defined by manifests.

---

## 5. lobo-aosp-platform — detailed structure

This is the main repository. It contains all Lobo-specific code.

```
lobo-aosp-platform/
│
├── vendor/lobo/                          ← Common code (shared across all hardware targets)
│   ├── Android.mk                        ← Registers rpi5_custom product with AOSP
│   ├── vendorsetup.sh                    ← Sourced by AOSP envsetup.sh automatically
│   │
│   ├── common/
│   │   ├── cpp/
│   │   │   ├── logging/                  ← liblobo_logging (C++ static library)
│   │   │   │   ├── Android.bp
│   │   │   │   ├── include/lobo/platform/logging/Logging.h
│   │   │   │   ├── src/Logging.cpp
│   │   │   │   └── tests/src/LoggingTest.cpp
│   │   │   ├── fan_math/                 ← liblobo_fan_math (C++ static library)
│   │   │   │   ├── Android.bp
│   │   │   │   ├── include/lobo/platform/fan_math/FanMath.h
│   │   │   │   ├── src/FanMath.cpp
│   │   │   │   └── tests/src/FanMathTest.cpp
│   │   │   └── types/                    ← liblobo_types_headers (header-only library)
│   │   │       ├── Android.bp
│   │   │       └── include/lobo/platform/types/CommonTypes.h
│   │   └── java/                         ← lobo-common-java (Kotlin/Java library)
│   │       ├── Android.bp
│   │       └── src/main/java/com/lobo/platform/common/
│   │           ├── fanmath/              ← FanMath API + implementation
│   │           ├── logging/              ← Logger API + implementation
│   │           └── types/               ← CommonTypes, Constants
│   │
│   ├── services/
│   │   ├── fancontrol/                   ← FanControlService (C++ daemon)
│   │   │   ├── Android.bp
│   │   │   ├── fancontrol.rc             ← init script (starts service at boot)
│   │   │   ├── aidl/                     ← IFanService.aidl (binder interface)
│   │   │   ├── core/                     ← FanService, FanController, ThermalConfig
│   │   │   ├── hal_bridge/               ← FanHalBridge (talks to hardware HAL)
│   │   │   ├── thermal/                  ← ThermalPolicy (temperature thresholds)
│   │   │   ├── sepolicy/                 ← SELinux policy for this service
│   │   │   └── tests/                    ← Unit tests
│   │   ├── fan_settings_service/         ← FanSettingsService (Android app, vendor)
│   │   │   ├── Android.bp
│   │   │   ├── AndroidManifest.xml
│   │   │   ├── fan_settings_service.rc   ← init script
│   │   │   ├── aidl/                     ← IFanSettingsService.aidl
│   │   │   ├── src/main/java/            ← Kotlin source
│   │   │   ├── res/                      ← Android resources
│   │   │   └── sepolicy/
│   │   └── name_service/                 ← NameService (C++ + Java, vendor)
│   │       ├── Android.bp
│   │       ├── name_service.rc
│   │       ├── aidl/                     ← INameService.aidl
│   │       ├── cpp/core/                 ← C++ service implementation
│   │       ├── cpp/client/               ← C++ client library
│   │       ├── java/                     ← Kotlin client library
│   │       └── sepolicy/
│   │
│   └── apps/
│       ├── system/MySystemApp/           ← Privileged system app (signed with platform key)
│       │   ├── Android.bp
│       │   ├── AndroidManifest.xml
│       │   ├── src/main/java/            ← Kotlin source (MVP pattern)
│       │   ├── res/
│       │   └── sepolicy/
│       └── user/MyUserApp/               ← Regular user app
│           ├── Android.bp
│           ├── AndroidManifest.xml
│           ├── src/main/java/            ← Kotlin source (MVP pattern)
│           └── res/
│
└── projects/rpi5_custom/                 ← RPi5-specific config (device-specific)
    ├── AndroidProducts.mk                ← Declares lunch targets for this product
    ├── rpi5_custom.mk                    ← Main product definition (inherits aosp_rpi5.mk)
    ├── device.mk                         ← Package list and file copies
    ├── BoardConfig.mk                    ← Board-level config (inherits rpi5 BoardConfig)
    └── hal/
        ├── interfaces/                   ← Abstract HAL interfaces (hardware-independent)
        │   ├── Android.bp
        │   └── include/lobo/platform/hal/
        │       ├── IFanHal.h             ← Abstract fan interface
        │       └── IThermalHal.h         ← Abstract thermal interface
        └── fan/                          ← RPi5-specific fan HAL implementation
            ├── Android.bp
            ├── include/lobo/platform/hal/rpi5/FanHalRpi5.h
            └── src/
                ├── FanHalRpi5.cpp        ← Fan control via GPIO
                └── FanHalGpio.cpp        ← GPIO pin control
```

---

## 6. How lobo-aosp-platform connects to AOSP

AOSP needs to "see" your code during the build. It does this by scanning the
`vendor/` directory for `Android.mk`, `Android.bp`, and `AndroidProducts.mk` files.

The connection is made by placing your code (via bind mounts) inside the AOSP tree's
`vendor/` directory:

```
raspi5-aosp/vendor/lobo/              ← AOSP sees this as a real directory
        ↕ (bind mount — same files)
lobo-aosp-platform/vendor/lobo/       ← actual files live here
```

When AOSP's build system scans `vendor/lobo/`, it finds:
- `Android.mk` → registers the `rpi5_custom` product
- `vendorsetup.sh` → sourced automatically, adds lunch combo
- All `Android.bp` files → builds your libraries, services, and apps

---

## 7. Bind mounts — what, why and how

### What is a bind mount?

A bind mount makes one directory appear at two filesystem paths simultaneously.
Both paths show the same files. Editing a file at either path changes it at both.

```bash
mount --bind /source/directory  /target/directory
```

### Why bind mounts (not symlinks)?

We tried symlinks first and hit two critical problems:

**Problem 1 — Soong scanner hung (50+ GB RAM)**
To make Soong follow symlinks you must set `ALLOW_BP_UNDER_SYMLINKS=true`.
This caused Soong to follow ALL symlinks in the tree — including `.repo/`
git-internal symlinks — and it scanned gigabytes of git objects indefinitely.

**Problem 2 — Duplicate BoardConfig.mk**
AOSP's product scanner runs `find -L vendor -maxdepth 4` which follows symlinks.
It found `BoardConfig.mk` at two paths simultaneously and crashed with a
"duplicate product" error.

**Why bind mounts solve both:**
Soong sees a real directory (not a symlink) — no special flags needed.
`find` sees a real directory — no duplication.

**Safety benefit:**
With bind mounts, deleting `raspi5-aosp/` only removes the empty mount point
directory. The actual files in `lobo-aosp-platform/` are completely untouched.

### How to set up bind mounts

Run these commands on the server after every fresh `repo sync`:

```bash
# Create empty mount point directories inside raspi5-aosp
mkdir -p /root/lobo-aosp/raspi5-aosp/vendor/lobo
mkdir -p /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom

# Bind mount lobo-aosp-platform into raspi5-aosp
sudo mount --bind \
    /root/lobo-aosp/lobo-aosp-platform/vendor/lobo \
    /root/lobo-aosp/raspi5-aosp/vendor/lobo

sudo mount --bind \
    /root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom \
    /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom
```

### Persist bind mounts across server reboots

Add to `/etc/fstab` so they are automatically remounted after a reboot:

```
/root/lobo-aosp/lobo-aosp-platform/vendor/lobo  /root/lobo-aosp/raspi5-aosp/vendor/lobo  none  bind  0  0
/root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom  /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom  none  bind  0  0
```

### Verify bind mounts are active

```bash
mount | grep lobo
```

### Remove bind mounts (before deleting raspi5-aosp)

```bash
sudo umount /root/lobo-aosp/raspi5-aosp/vendor/lobo
sudo umount /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom
```

---

## 8. Product inheritance chain

When you run `lunch rpi5_custom-trunk_staging-userdebug`, Android loads
product configuration in this chain:

```
lunch rpi5_custom-trunk_staging-userdebug
    │
    └── vendor/projects/rpi5_custom/rpi5_custom.mk       ← YOUR product definition
            │
            ├── device/brcm/rpi5/aosp_rpi5.mk            ← raspberry-vanilla RPi5 product
            │       │
            │       └── device/brcm/rpi5/device.mk       ← RPi5 packages and overlays
            │               │
            │               └── AOSP base (full_base.mk, etc.)
            │
            └── vendor/projects/rpi5_custom/device.mk    ← YOUR packages + PRODUCT_COPY_FILES
```

**What this means:**
`rpi5_custom.mk` inherits two things:
1. `aosp_rpi5.mk` — gets kernel, GPU drivers, WiFi, audio, full Android framework from raspberry-vanilla
2. `device.mk` — adds your Lobo `PRODUCT_PACKAGES` and `PRODUCT_COPY_FILES` (init `.rc` files)

`$(call inherit-product, ...)` always uses paths relative to the AOSP tree root (`$TOP`).
After the bind mount, `lobo-aosp-platform/projects/rpi5_custom/` is visible as
`vendor/projects/rpi5_custom/` from `$TOP`, so the path is exactly:
```makefile
$(call inherit-product, vendor/projects/rpi5_custom/device.mk)
```

You never modify `device/brcm/rpi5/` — that is raspberry-vanilla's code.

---

## 9. Build system files explained

### `vendorsetup.sh`
**Location:** `vendor/lobo/vendorsetup.sh`
**Sourced by:** AOSP automatically during `source build/envsetup.sh`
**Purpose:** Entry point that connects your platform code to the AOSP build system.
Currently minimal — bind mounts make special flags unnecessary.

### `Android.mk`
**Location:** `vendor/lobo/Android.mk`
**Purpose:** Target-agnostic hook: auto-registers every board under `projects/`
that follows the naming rule `projects/<name>/<name>.mk`, and includes each
`projects/*/AndroidProducts.mk` for lunch combos. You do **not** list RPi5 vs VIM3
here — add a new directory under `projects/` instead.

### `AndroidProducts.mk`
**Location:** `projects/rpi5_custom/AndroidProducts.mk` (one per board)
**Purpose:** Appends `COMMON_LUNCH_CHOICES` for that board only. Product makefiles
are registered by `vendor/lobo/Android.mk`, not in this file.
```makefile
COMMON_LUNCH_CHOICES += \
    rpi5_custom-trunk_staging-userdebug \
    rpi5_custom-trunk_staging-user \
    rpi5_custom-trunk_staging-eng
```

### `rpi5_custom.mk`
**Location:** `projects/rpi5_custom/rpi5_custom.mk`
**Purpose:** Main product definition. Inherits two products in order:
1. `device/brcm/rpi5/aosp_rpi5.mk` — full raspberry-vanilla RPi5 base
2. `vendor/projects/rpi5_custom/device.mk` — Lobo packages and `.rc` files

Sets `PRODUCT_NAME`, `PRODUCT_BRAND`, `PRODUCT_MODEL`, `PRODUCT_MANUFACTURER` here only.
**Does NOT contain `PRODUCT_PACKAGES`** — that belongs exclusively in `device.mk`.

### `device.mk`
**Location:** `projects/rpi5_custom/device.mk`
**Purpose:** **Single place** for Lobo `PRODUCT_PACKAGES` and `PRODUCT_COPY_FILES`
(init `.rc` files). It does **not** re-inherit `device/brcm/rpi5/device.mk` (that
already comes from `aosp_rpi5.mk`). **Why:** avoids duplicating the same package
list in two files and ensures `PRODUCT_COPY_FILES` is actually applied (`device.mk`
must be pulled in via `$(call inherit-product, vendor/projects/rpi5_custom/device.mk)`
from `rpi5_custom.mk`). **How:** add packages or copy rules here; keep `PRODUCT_NAME`
in `rpi5_custom.mk` only.

### `BoardConfig.mk`
**Location:** `projects/rpi5_custom/BoardConfig.mk`
**Purpose:** Board-level hardware configuration. Inherits all RPi5 board settings
from `device/brcm/rpi5/BoardConfig.mk` and adds only Lobo-specific overrides
(e.g. loading the `lobo_fan.ko` kernel module).
**Important:** Never set `PRODUCT_NAME` here — only set it in `rpi5_custom.mk`.

### `Android.bp` files
Used by Soong (AOSP's modern build system) to define each module:
- `cc_binary` — C++ executable (FanControlService, NameService)
- `cc_library_static` — C++ static library (liblobo_logging, liblobo_fan_math)
- `cc_library_headers` — header-only library (liblobo_types_headers, lobo_hal_interfaces)
- `android_app` — Android application (FanSettingsService, MySystemApp, MyUserApp)
- `java_library` — Java/Kotlin library (lobo-common-java)
- `aidl_interface` — AIDL Binder interface definition

### `.rc` files (init scripts)
Files like `fancontrol.rc` and `name_service.rc` tell Android's `init` process
to start your services at boot. They are copied to `/vendor/etc/init/` on the device.

### `.te` files (SELinux policy)
Each vendor service needs a SELinux type enforcement policy file.
Without it, Android's security system blocks the service from starting.

---

## 10. Setting up from scratch

### Step 1 — Connect to server
```bash
ssh lobo-hetzner
tmux new -s aosp        # or: tmux attach -t aosp
```

### Step 2 — Set build mode
```bash
~/scripts/build-mode.sh
```

### Step 3 — Clone lobo-aosp-platform (if not already present)
```bash
cd /root/lobo-aosp
git clone https://github.com/francissunillobo/lobo-aosp-platform.git
```

### Step 4 — Initialize AOSP repo (Android 16)
```bash
cd /root/lobo-aosp/raspi5-aosp
repo init -u https://android.googlesource.com/platform/manifest \
          -b android-16.0.0_r3 --depth=1
```
`--depth=1` does a shallow clone — saves ~30 GB, no full git history needed.

### Step 5 — Add raspberry-vanilla manifests
```bash
curl -o .repo/local_manifests/manifest_brcm_rpi.xml -L \
  https://raw.githubusercontent.com/raspberry-vanilla/android_local_manifest/android-16.0.0_r3/manifest_brcm_rpi.xml \
  --create-dirs

curl -o .repo/local_manifests/remove_projects.xml -L \
  https://raw.githubusercontent.com/raspberry-vanilla/android_local_manifest/android-16.0.0_r3/remove_projects.xml
```

### Step 6 — Sync AOSP source
```bash
repo sync -c -j16 --no-tags --no-clone-bundle 2>&1 | tee ~/sync-android16.log
```
Takes ~30 minutes on the Hetzner server.

### Step 7 — Set up bind mounts
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

### Step 8 — Source build environment
```bash
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
```

### Step 9 — Select build target
```bash
lunch rpi5_custom-trunk_staging-userdebug
```

### Step 10 — Build
```bash
make bootimage systemimage vendorimage -j$(nproc) 2>&1 | tee ~/build-android16.log
```

### Step 11 — Create flashable image
```bash
./rpi5-mkimg.sh
```
Output: `out/target/product/rpi5/RaspberryVanillaAOSP16-<date>-rpi5_car.img`

---

## 11. Day-to-day build commands

```bash
# Always run these first in a new shell session
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
lunch rpi5_custom-trunk_staging-userdebug

# Build only the 3 images needed for RPi5
make bootimage systemimage vendorimage -j$(nproc) 2>&1 | tee ~/build-android16.log

# Build specific modules only (faster for development)
make FanControlService NameService -j$(nproc)
make MySystemApp MyUserApp -j$(nproc)
make FanSettingsService -j$(nproc)

# Check for errors in build log
grep -E "error:|FAILED" ~/build-android16.log | head -50

# Monitor build progress in another terminal
tail -f ~/build-android16.log

# Create flashable SD card image after build
./rpi5-mkimg.sh

# Disk and RAM usage
df -h
free -h
```

---

## 12. Flashing to Raspberry Pi 5

### Copy image to local machine
Run on your **local Linux machine** (not the server):
```bash
rsync -avz --progress \
  root@<server-ip>:/root/lobo-aosp/raspi5-aosp/out/target/product/rpi5/RaspberryVanillaAOSP16-*.img \
  ~/Downloads/
```

`rsync` is preferred over `scp` because it can resume a failed transfer.

### Identify SD card device
Insert SD card, then run:
```bash
lsblk
```
Look for your SD card — typically `/dev/sdb` or `/dev/mmcblk0`.
**Double-check before flashing — wrong device = data loss.**

### Flash the image
```bash
sudo dd if=RaspberryVanillaAOSP16-*.img of=/dev/sdX bs=4M status=progress
sync
```
Replace `/dev/sdX` with your actual SD card device.

### What the image contains
The single `.img` file contains all partitions:

| Partition | Contents |
|---|---|
| `boot` | Linux kernel + ramdisk (first thing RPi5 loads) |
| `system` | Android framework, apps, services |
| `vendor` | Hardware drivers, HALs (GPU, WiFi, camera, fan) |

The ramdisk is bundled inside `boot.img`. You do not need to flash partitions
separately — the `.img` file handles everything.

---

## 13. Adding a new hardware target (e.g. VIM3)

The architecture is designed for this. Steps:

1. Create `projects/vim3_custom/` in `lobo-aosp-platform` (similar to `rpi5_custom/`):
   include `vim3_custom.mk`, `AndroidProducts.mk`, `BoardConfig.mk`, `device.mk`,
   and VIM3-specific HAL under `hal/`. Follow the naming rule
   `projects/vim3_custom/vim3_custom.mk` — `vendor/lobo/Android.mk` picks it up
   automatically; you do **not** edit `vendor/lobo/` to add a new board.
2. Sync a new AOSP tree: `/root/lobo-aosp/vim3-aosp/`
3. Create bind mounts:
```bash
mkdir -p /root/lobo-aosp/vim3-aosp/vendor/lobo
mkdir -p /root/lobo-aosp/vim3-aosp/vendor/projects/vim3_custom

sudo mount --bind \
    /root/lobo-aosp/lobo-aosp-platform/vendor/lobo \
    /root/lobo-aosp/vim3-aosp/vendor/lobo

sudo mount --bind \
    /root/lobo-aosp/lobo-aosp-platform/projects/vim3_custom \
    /root/lobo-aosp/vim3-aosp/vendor/projects/vim3_custom
```

`vendor/lobo/` (common code) is shared automatically.
`projects/vim3_custom/` contains only VIM3-specific board config and HAL.
`raspi5-aosp` never sees `vim3_custom/` and vice versa.

---

## 14. Known issues and lessons learned

### Do not put lobo-aosp-platform inside vendor/
Placing it at `vendor/lobo_platform` caused AOSP's `find -L vendor -maxdepth 4`
to find `BoardConfig.mk` at two paths, crashing the build with a duplicate product error.
**Rule:** Always keep `lobo-aosp-platform` outside all AOSP trees.

### Do not use ALLOW_BP_UNDER_SYMLINKS=true
Setting this flag caused Soong's file scanner to follow `.repo/` git-internal
symlinks and consume 50+ GB RAM, hanging indefinitely.
**Rule:** Use bind mounts instead of symlinks — Soong sees real directories.

### Do not set PRODUCT_NAME in BoardConfig.mk
`PRODUCT_NAME` is a read-only variable by the time `BoardConfig.mk` is processed.
**Rule:** Set `PRODUCT_NAME` only in `rpi5_custom.mk`.

### Use PRODUCT_COPY_FILES for .rc files (not init_rc in Android.bp)
Using `init_rc` in an `Android.bp` `cc_binary` block causes a Soong/Make bridge
conflict where the file gets installed twice.
**Rule:** Always use `PRODUCT_COPY_FILES` in `device.mk` to install `.rc` files.

### Android 15 lunch format changed in Android 15+
Android 15+ requires three-part lunch targets: `product-release-variant`.
`rpi5_custom-userdebug` fails. Use `rpi5_custom-trunk_staging-userdebug`.

### android_library requires AndroidManifest.xml
If a module has no UI resources, use `java_library` not `android_library`.
`android_library` always requires an `AndroidManifest.xml`.

### Build log is essential
Always use `tee` to save build output:
```bash
make ... 2>&1 | tee ~/build-android16.log
```
`2>&1` merges error output into standard output so both go to the log file.
Without it, errors appear on screen but are not saved.
