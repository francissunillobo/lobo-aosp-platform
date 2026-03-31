# `rpi5_custom_car` Project — `projects/rpi5_custom_car/`

**What:** The Android Automotive product configuration for Raspberry Pi 5.
It defines a complete `lunch` target (`rpi5_custom_car-trunk_staging-userdebug`)
that builds the full Android Automotive OS stack on top of the RPi5 community
board support package, with Lobo platform services added on top.

**Why:** Android Automotive (`aosp_rpi5_car`) is a separate product from standard
Android (`aosp_rpi5`). It includes the car-specific stack — CarService, Vehicle HAL,
EVS cameras, CAN bus — which is not present in the standard build. A separate project
folder keeps car-specific config isolated from the standard `rpi5_custom` project,
so `vendor/lobo/Android.mk` stays hardware-independent and adding VIM3 or other
targets never requires editing the car project.

**How:** Four files, each with a single responsibility. Read them in the order below —
it follows the build system's discovery and execution order.

---

## Directory Layout

```
projects/rpi5_custom_car/
├── AndroidProducts.mk    ← Step 1: AOSP discovers this product and its lunch targets
├── rpi5_custom_car.mk    ← Step 2: top-level product definition, pulls in all software
├── BoardConfig.mk        ← Step 3: hardware config (CPU, kernel, partitions)
└── device.mk             ← Step 4: Lobo-only additions (packages + files to install)
```

These files live in `lobo-aosp-platform/projects/rpi5_custom_car/` and are
**bind-mounted** into the AOSP tree at `vendor/projects/rpi5_custom_car/` so the
build system can find them without modifying the AOSP tree.

---

## File 1 — `AndroidProducts.mk` — Product Registration

### What
Registers the product with AOSP's build system. Tells it:
- Where the main product makefile lives (`PRODUCT_MAKEFILES`)
- What `lunch` targets are available (`COMMON_LUNCH_CHOICES`)

### Why
AOSP's Soong scanner finds every `AndroidProducts.mk` in the source tree and writes
their paths to `out/.module_paths/AndroidProducts.mk.list`. Then
`build/make/core/product_config.mk` reads that list and calls `_read-ap-file` for
each — resetting `PRODUCT_MAKEFILES` and `COMMON_LUNCH_CHOICES` before including
each file, and validating that every lunch target maps to a declared product.

Without this file, `rpi5_custom_car` never appears in `lunch`.

### How
```makefile
PRODUCT_MAKEFILES += $(LOCAL_DIR)/rpi5_custom_car.mk
```
`$(LOCAL_DIR)` is set to `vendor/projects/rpi5_custom_car` by `_read-ap-file` before
including this file. Resolves to `vendor/projects/rpi5_custom_car/rpi5_custom_car.mk`.

```makefile
COMMON_LUNCH_CHOICES += \
    rpi5_custom_car-trunk_staging-userdebug \
    rpi5_custom_car-trunk_staging-user \
    rpi5_custom_car-trunk_staging-eng
```
Three build variants. `trunk_staging` is the Android 16 release codename.
Format: `<product>-<release>-<variant>` (Android 15+ requires all three parts).

---

## File 2 — `rpi5_custom_car.mk` — Main Product Makefile

### What
The top-level product definition. Sets the product identity and pulls in all software
layers via two `inherit-product` calls.

### Why
Every Android product must have exactly one `.mk` file that sets `PRODUCT_NAME`,
`PRODUCT_DEVICE`, `PRODUCT_BRAND`, etc. The build system uses these to name the
output images and to select the correct device tree during the build.

### How

```makefile
$(call inherit-product, device/brcm/rpi5/aosp_rpi5_car.mk)
```
Pulls in the **full upstream Android Automotive stack** maintained by the RPi5
community (`device/brcm/rpi5/`):

| What it adds | Purpose |
|---|---|
| `car.mk` | Android Automotive base (CarService, CarSettings, CarShell) |
| Vehicle HAL | Abstracts the car bus (CAN, OBD) for upper Android layers |
| EVS (Exterior View System) | Camera HAL for surround-view / rear-view cameras |
| CAN bus support | Communicates with vehicle ECUs |
| `device/brcm/rpi5/device.mk` | GPU drivers, WiFi, BT, audio, kernel modules |

```makefile
$(call inherit-product, vendor/projects/rpi5_custom_car/device.mk)
```
Pulls in **Lobo-only additions** — our packages and files. Kept separate so the
Lobo diff is always visible in one place (see `device.mk` below).

> **Note:** `$(call inherit-product, ...)` always uses paths relative to the
> **AOSP tree root** (`$TOP`), not relative to this file. So
> `vendor/projects/rpi5_custom_car/device.mk` resolves to the bind-mounted file at
> `/root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom_car/device.mk`.

```makefile
PRODUCT_NAME         := rpi5_custom_car
PRODUCT_DEVICE       := rpi5_custom_car
PRODUCT_BRAND        := Lobo
PRODUCT_MODEL        := Lobo RPi5 Car
PRODUCT_MANUFACTURER := Lobo
```
`PRODUCT_NAME` must match the filename (`rpi5_custom_car.mk`) — Android 16 enforces
this. `PRODUCT_DEVICE` tells the build system which `BoardConfig.mk` to use.

### Inheritance chain

```
rpi5_custom_car.mk
  └── device/brcm/rpi5/aosp_rpi5_car.mk
        └── device/brcm/rpi5/device.mk   (GPU, WiFi, BT, audio, kernel modules)
              └── device/brcm/rpi5/BoardConfig.mk (CPU arch, partitions, bootloader)
  └── vendor/projects/rpi5_custom_car/device.mk  (Lobo packages)
```

> **Do NOT** call `inherit-product` for `device/brcm/rpi5/device.mk` directly in
> `device.mk`. That chain is already applied by `aosp_rpi5_car.mk`. Inheriting it
> again would duplicate all packages and cause build errors.

---

## File 3 — `BoardConfig.mk` — Hardware Configuration

### What
Board-level build settings: CPU architecture flags, kernel image format, partition
sizes, bootloader type. Used by Make (not Soong) to configure the kernel build and
partition layout.

### Why
The build system needs to know the hardware characteristics before compiling anything.
Without `BoardConfig.mk`, it does not know the target CPU ISA, how large to make each
partition, or which bootloader to generate.

### How

```makefile
-include device/brcm/rpi5/BoardConfig.mk
```

The `-` prefix means: **include if the file exists; silently skip if not**.
This protects the build if the RPi5 community tree hasn't been synced yet — the
build will still parse cleanly.

We inherit **everything** from the upstream RPi5 board config as a base, then add
Lobo-specific overrides below it.

### Current Lobo overrides

#### `BOARD_VENDOR_SEPOLICY_DIRS`

```makefile
BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/services/calculator/sepolicy
```

**What:** Adds the calculator service SELinux policy directory to the build.

**Why:** Without this, the build ignores our `.te`, `file_contexts`, and
`service_contexts` files entirely. The result is that the `calculatord` binary
gets the generic `vendor_file` SELinux label instead of `calculatord_exec`, and
the service name gets `default_android_service` instead of `calculatord_service`.
Even in permissive mode, `init` refuses to start a service whose domain type is
undefined — so `calculatord` never starts.

**Rule:** Every new vendor service that has a `sepolicy/` folder must be added here.

#### `BOARD_AVB_ENABLE := false`

```makefile
BOARD_AVB_ENABLE := false
```

**What:** Disables Android Verified Boot (AVB).

**Why:** The RPi5 bootloader does not support AVB. When AVB is enabled, the
vendor partition is mounted read-only and locked — `adb remount` fails with
`Device must be bootloader unlocked`, and even `mount -o rw,remount /vendor`
fails with `not user mountable in fstab`. This makes it impossible to push
updated files to `/vendor` during development without reflashing the full SD card.
Disabling AVB removes the verity enforcement, allowing `adb remount` to work.

**Impact:** Disabling AVB means the bootloader does not verify partition signatures
at boot. This is acceptable for a development board (RPi5) but should be
re-evaluated before any production deployment.

**How to use after disabling AVB:**
```bash
adb root
adb remount
adb push <file> /vendor/...
adb reboot
```

#### Future additions

```makefile
# Example future additions:
BOARD_VENDOR_KERNEL_MODULES_LOAD += lobo_fan.ko   # load fan driver at boot
BOARD_KERNEL_CMDLINE += lobo_debug=1              # custom kernel param
```

---

## File 4 — `device.mk` — Lobo Additions

### What
The single place that lists everything Lobo adds on top of the upstream car product.
Currently: the calculator service daemon and its init script.

### Why
Keeping Lobo additions here (not in `rpi5_custom_car.mk`) means the diff between
upstream and Lobo is always visible in one file. Reviewers and future maintainers
know exactly what Lobo contributes without reading the entire upstream inheritance
chain.

### How

```makefile
PRODUCT_PACKAGES += \
    calculatord
```
Adds the `calculatord` binary (defined in
`vendor/lobo/services/calculator/Android.bp`) to the build. Soong compiles it and
the build system installs it to `/vendor/bin/calculatord` in `vendor.img`.

```makefile
PRODUCT_COPY_FILES += \
    vendor/lobo/services/calculator/calculator.rc:\
    $(TARGET_COPY_OUT_VENDOR)/etc/init/calculator.rc
```
Copies the init script to `/vendor/etc/init/calculator.rc`. Android's `init` process
scans that directory at boot and automatically starts `calculatord`.

> **Why `PRODUCT_COPY_FILES` instead of `init_rc:` in Android.bp?**
> Using `init_rc:` in a Soong module inside a vendor path causes a conflict with
> the Soong/Make bridge when `PRODUCT_PACKAGES` installs the module. `PRODUCT_COPY_FILES`
> in `device.mk` is the explicit, conflict-free alternative.

---

## How the Files Link Together

```
AOSP build system
  │
  ├── Soong scanner finds vendor/projects/rpi5_custom_car/AndroidProducts.mk
  │     → written to out/.module_paths/AndroidProducts.mk.list
  │
  ├── product_config.mk calls _read-ap-file:
  │     PRODUCT_MAKEFILES := vendor/projects/rpi5_custom_car/rpi5_custom_car.mk
  │     COMMON_LUNCH_CHOICES := rpi5_custom_car-trunk_staging-{userdebug,user,eng}
  │
  ├── lunch rpi5_custom_car-trunk_staging-userdebug
  │     → selects rpi5_custom_car.mk
  │
  ├── rpi5_custom_car.mk
  │     ├── inherit device/brcm/rpi5/aosp_rpi5_car.mk   (full car stack)
  │     └── inherit vendor/projects/rpi5_custom_car/device.mk
  │           ├── PRODUCT_PACKAGES += calculatord
  │           └── PRODUCT_COPY_FILES += calculator.rc
  │
  ├── BoardConfig.mk
  │     └── -include device/brcm/rpi5/BoardConfig.mk
  │           (CPU ISA, partition sizes, kernel format, bootloader)
  │
  └── make bootimage systemimage vendorimage
        → out/target/product/rpi5_custom_car/
              boot.img      ← kernel + ramdisk
              system.img    ← Android Automotive framework + CarService
              vendor.img    ← calculatord + calculator.rc + HAL libraries
```

---

## Runtime Flow After Flashing

```
RPi5 powers on
  → bootloader loads boot.img
  → kernel boots, mounts vendor.img
  → init reads /vendor/etc/init/calculator.rc
  → starts /vendor/bin/calculatord
  → calculatord registers "com.lobo.platform.calculator.ICalculatorService"
     in ServiceManager
  → Android Automotive UI starts (CarLauncher, CarSettings, etc.)
  → apps can call calculatord via Binder IPC
```

---

## Relationship to `rpi5_custom`

| | `rpi5_custom` | `rpi5_custom_car` |
|---|---|---|
| Upstream base | `aosp_rpi5.mk` (standard Android) | `aosp_rpi5_car.mk` (Android Automotive) |
| Car stack | No | Yes (CarService, Vehicle HAL, EVS, CAN) |
| Lobo packages | `calculatord` | `calculatord` |
| HAL | `projects/rpi5_custom/hal/` | inherits same (shared — future: move to `rpi5_common_hal/`) |
| Lunch target | `rpi5_custom-trunk_staging-userdebug` | `rpi5_custom_car-trunk_staging-userdebug` |

---

## Adding More Lobo Packages

To add a new Lobo service or app to the car build, edit `device.mk` only:

```makefile
PRODUCT_PACKAGES += \
    calculatord \
    CalculatorClientApp    ← add more packages here

PRODUCT_COPY_FILES += \
    vendor/lobo/services/calculator/calculator.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/calculator.rc
```

Do **not** add packages to `rpi5_custom_car.mk` — that file only sets identity and
inherits product layers.

---

## Build and Flash Commands

```bash
# On the Hetzner server
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
lunch rpi5_custom_car-trunk_staging-userdebug

# Full build
make bootimage systemimage vendorimage -j$(nproc) 2>&1 | tee ~/build-android16-car.log

# Check for errors
grep -E "error:|FAILED" ~/build-android16-car.log | head -50

# Output images
ls out/target/product/rpi5_custom_car/*.img
```

---

See also:
- [PROJECT_GUIDE.md](PROJECT_GUIDE.md) — full platform architecture and setup
- [CALCULATOR_SERVICE.md](CALCULATOR_SERVICE.md) — calculator service internals
- [SETUP.md](SETUP.md) — server setup, bind mounts, day-to-day commands

**Author:** Francis Lobo · **Project:** lobo-aosp-platform
