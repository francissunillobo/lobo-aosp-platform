# `projects/` — board-specific Android products

## What

This directory holds **one subdirectory per hardware target** (e.g. `rpi5_custom`, future `vim3_custom`). Each board owns its **product makefiles**, **board config**, and **target-specific HAL** code. Shared Lobo code lives under **`vendor/lobo/`**, not here.

---

## Why

- **Separation:** Keeps `vendor/lobo/` free of RPi5 vs Khadas–specific logic so the same services and apps compile for every target.
- **Multi-tree builds:** Each AOSP tree bind-mounts only **its** `projects/<name>/` (e.g. `raspi5-aosp` mounts `rpi5_custom` only), so Soong never scans unrelated boards.
- **Registration:** Soong discovers `projects/*/AndroidProducts.mk` and lists them in **`out/.module_paths/AndroidProducts.mk.list`**; **`product_config.mk`** loads each file once (sets **`LOCAL_DIR`**, validates). Do **not** include `AndroidProducts.mk` from **`vendor/lobo/Android.mk`** — that duplicates processing and can break `lunch`. Each `AndroidProducts.mk` must still declare **`PRODUCT_MAKEFILES`** + **`COMMON_LUNCH_CHOICES`** together.

---

## How

### Layout convention

| Path | Role |
|------|------|
| `projects/<name>/<name>.mk` | Main product makefile (`inherit-product`, `PRODUCT_PACKAGES`, …) |
| `projects/<name>/AndroidProducts.mk` | `PRODUCT_MAKEFILES +=` + `COMMON_LUNCH_CHOICES +=` (both required in one file) |
| `projects/<name>/BoardConfig.mk`, `device.mk`, `hal/` | Board config and target-specific HAL |

### Rules

- Use **only directories** under `projects/` (no stray loose files at this level).
- Name the main makefile **`/<name>.mk`** the same as the directory **`<name>`**, and reference it from **`AndroidProducts.mk`** via `$(LOCAL_DIR)/<name>.mk`.

### Adding a new board (e.g. VIM3)

1. Create `projects/vim3_custom/` with `vim3_custom.mk`, `AndroidProducts.mk`, and the rest (see `docs/PROJECT_GUIDE.md` §13).
2. In the **VIM3** AOSP tree, bind-mount that folder to `vendor/projects/vim3_custom` (see `docs/SETUP.md`).
3. `source build/envsetup.sh` → `lunch <vim3_custom-target>` → build.

---

See also: `docs/DOCUMENTATION_STYLE.md` (What / Why / How for all docs), `docs/PROJECT_GUIDE.md`.
