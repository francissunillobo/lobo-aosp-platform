#!/usr/bin/env bash
# cleanup-stale-lobo-vendor-dirs.sh
#
# What: Deletes a small set of *known* legacy directories under vendor/lobo
#       that no longer have an Android.bp but still confuse Soong's glob
#       tracking after refactors.
#
# Why: After moving e.g. thermal Java from common/thermalcontrol to client/thermalcontrol,
#      an empty tree or old copy can remain on disk. Soong may print
#      "One culprit glob: vendor/lobo/common/thermalcontrol/..." until removed.
#
# Currently removes:
#   $VLOB/common/thermalcontrol
#
# How:
#   ./cleanup-stale-lobo-vendor-dirs.sh /root/lobo-aosp/lobo-aosp-platform/vendor/lobo
#   ./cleanup-stale-lobo-vendor-dirs.sh /root/lobo-aosp/raspi5-aosp
#   (second form uses .../vendor/lobo if present)
#   From prepare-rpi5-custom-car-build.sh only when:
#     LOBO_CLEANUP_STALE_VENDOR_DIRS=1 ./prepare-rpi5-custom-car-build.sh

set -euo pipefail

arg="${1:-}"
if [[ -z "$arg" ]]; then
  echo "Usage: $0 <path-to-vendor/lobo OR path-to-raspi5-aosp>" >&2
  exit 1
fi

if [[ -d "$arg/vendor/lobo" ]]; then
  VLOB="$arg/vendor/lobo"
elif [[ -f "$arg/Android.mk" && -d "$arg/common" ]]; then
  # Already .../vendor/lobo
  VLOB="$arg"
else
  echo "Cannot find vendor/lobo: pass raspi5-aosp root or vendor/lobo path" >&2
  exit 1
fi

STALE=(
  "$VLOB/common/thermalcontrol"
)

for d in "${STALE[@]}"; do
  if [[ -d "$d" ]]; then
    echo "Removing stale: $d"
    rm -rf "$d"
  fi
done

echo "OK: stale vendor/lobo paths cleaned (if any existed)."
