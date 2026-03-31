#!/bin/bash
# envsetup-rpi5-custom-car.sh
#
# What: Same as a manual session:
#         cd /root/lobo-aosp/raspi5-aosp
#         source build/envsetup.sh
#         lunch rpi5_custom_car-trunk_staging-userdebug
#
# Why: One place to set AOSP_ROOT / LUNCH_TARGET for rpi5_custom_car builds.
#
# How:
#   Source it so lunch and OUT_DIR stay in your current shell (recommended):
#     source /path/to/lobo-aosp-platform/scripts/envsetup-rpi5-custom-car.sh
#   Or run (subshell — use only if you chain a command that reads env from the same process):
#     ./envsetup-rpi5-custom-car.sh
#
# Override defaults:
#   AOSP_ROOT=/other/raspi5-aosp LUNCH_TARGET=rpi5_custom_car-trunk_staging-userdebug \
#     source ./envsetup-rpi5-custom-car.sh

set -euo pipefail

AOSP_ROOT="${AOSP_ROOT:-/root/lobo-aosp/raspi5-aosp}"
LUNCH_TARGET="${LUNCH_TARGET:-rpi5_custom_car-trunk_staging-userdebug}"

cd "$AOSP_ROOT"
# AOSP build/envsetup.sh touches unset vars (e.g. TOP) before defining them;
# our `set -u` would abort — disable nounset only for envsetup + lunch.
set +u
# shellcheck source=/dev/null
source build/envsetup.sh
lunch "$LUNCH_TARGET"

