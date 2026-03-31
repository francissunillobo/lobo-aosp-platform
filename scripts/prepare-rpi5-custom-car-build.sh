#!/bin/bash
# prepare-rpi5-custom-car-build.sh
#
# What: Enters the AOSP tree, loads build/envsetup.sh, runs lunch for
#       rpi5_custom_car, then deletes stale *.img files under the product
#       output directory so the next full build regenerates images cleanly.
#
# Why: Old *.img files in out/target/product/<product>/ can confuse flashing
#      workflows or leave you looking at an outdated artifact after a partial
#      build. Removing them before `make` avoids that.
#
# How:
#   ./prepare-rpi5-custom-car-build.sh
#     → then runs: make -j$(nproc) 2>&1 | tee ~/build-rpi5_custom_car.log
#   ./prepare-rpi5-custom-car-build.sh make -j"$(nproc)" modules-only …
#     → runs your command instead of the default full build + tee
#   BUILD_LOG=/tmp/my.log ./prepare-rpi5-custom-car-build.sh
#     → same make, different log path
#
# Optional stale vendor/lobo cleanup (removes legacy, refactor leftovers):
#   LOBO_CLEANUP_STALE_VENDOR_DIRS=1 ./prepare-rpi5-custom-car-build.sh
# Significance: this is an OPT-IN safety step to prevent Soong from
# complaining about stale on-disk globs (for Lobo we currently delete only
# $VLOB/common/thermalcontrol via cleanup-stale-lobo-vendor-dirs.sh).
#
# Optional stale product image cleanup (removes old *.img artifacts before
# flashing workflows):
#   LOBO_CLEANUP_STALE_PRODUCT_IMAGES=1 ./prepare-rpi5-custom-car-build.sh
# By default this flag is enabled; set it to `0` to disable.
#
# Env-only (no *.img cleanup): see envsetup-rpi5-custom-car.sh
#
# Override paths with env vars if your tree differs:
#   AOSP_ROOT=/path/to/raspi5-aosp LUNCH_TARGET=rpi5_custom_car-trunk_staging-userdebug \
#     ./prepare-rpi5-custom-car-build.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=/dev/null
source "${SCRIPT_DIR}/envsetup-rpi5-custom-car.sh"

# Stale vendor/lobo dirs (opt-in): avoids Soong glob noise after refactors.
# Off by default; enable only when you see Soong "culprit glob" warnings.
if [[ "${LOBO_CLEANUP_STALE_VENDOR_DIRS:-}" == "1" ]]; then
  if [[ -x "${SCRIPT_DIR}/cleanup-stale-lobo-vendor-dirs.sh" ]]; then
    "${SCRIPT_DIR}/cleanup-stale-lobo-vendor-dirs.sh" "${AOSP_ROOT}" || true
  else
    echo "warning: LOBO_CLEANUP_STALE_VENDOR_DIRS=1 but cleanup-stale-lobo-vendor-dirs.sh is missing or not executable" >&2
  fi
fi

# Stale *.img artifacts from previous builds can lead to confusing flashing
# (especially after partial builds).
# Default: enabled; disable with LOBO_CLEANUP_STALE_PRODUCT_IMAGES=0.
LOBO_CLEANUP_STALE_PRODUCT_IMAGES="${LOBO_CLEANUP_STALE_PRODUCT_IMAGES:-1}"
if [[ "${LOBO_CLEANUP_STALE_PRODUCT_IMAGES}" == "1" ]]; then
  if [[ -n "${ANDROID_PRODUCT_OUT:-}" && -d "${ANDROID_PRODUCT_OUT}" ]]; then
    shopt -s nullglob
    echo "Removing stale product images under: ${ANDROID_PRODUCT_OUT}"
    rm -f "${ANDROID_PRODUCT_OUT}"/*.img || true
    shopt -u nullglob
  else
    echo "warning: LOBO_CLEANUP_STALE_PRODUCT_IMAGES=1 but ANDROID_PRODUCT_OUT is missing/unset" >&2
  fi
fi

if [ "$#" -gt 0 ]; then
  exec "$@"
fi

BUILD_LOG="${BUILD_LOG:-${HOME}/build-rpi5_custom_car.log}"
echo "Starting full build: make -j$(nproc) 2>&1 | tee ${BUILD_LOG}"
make -j"$(nproc)" 2>&1 | tee "${BUILD_LOG}"
