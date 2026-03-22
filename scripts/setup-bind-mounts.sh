#!/bin/bash
# setup-bind-mounts.sh
# Sets up bind mounts to connect lobo-aosp-platform into an AOSP tree.
#
# What:
#   Creates an empty mount point directory in the AOSP tree and bind-mounts
#   the corresponding project folder from lobo-aosp-platform into it.
#   Also bind-mounts vendor/lobo/ (common code, shared across all projects).
#
# Why:
#   lobo-aosp-platform lives outside all AOSP trees. Bind mounts make it
#   visible to the AOSP build system as a real directory (not a symlink),
#   avoiding Soong scanner issues and duplicate BoardConfig.mk errors.
#   Deleting an AOSP tree only removes the empty mount point — the actual
#   source files in lobo-aosp-platform are never affected.
#
# How:
#   Run once per project per AOSP tree. Safe to run multiple times —
#   skips already-mounted targets. Add fstab entries to persist across reboots.
#
# Usage:
#   ./scripts/setup-bind-mounts.sh <aosp-tree> <project>
#
# Current projects in lobo-aosp-platform:
#   rpi5_custom     — standard Android (inherits aosp_rpi5.mk)
#   rpi5_custom_car — Android Automotive (inherits aosp_rpi5_car.mk)
#
# Examples — run both for a full raspi5-aosp setup:
#   ./scripts/setup-bind-mounts.sh raspi5-aosp rpi5_custom
#   ./scripts/setup-bind-mounts.sh raspi5-aosp rpi5_custom_car
#
# Future VIM3 example:
#   ./scripts/setup-bind-mounts.sh vim3-aosp vim3_custom

set -e

PLATFORM_DIR="$(cd "$(dirname "$0")/.." && pwd)"
LOBO_ROOT="$(cd "$PLATFORM_DIR/.." && pwd)"

AOSP_TREE="$1"
PROJECT="$2"

# Validate arguments
if [ -z "$AOSP_TREE" ] || [ -z "$PROJECT" ]; then
    echo "Usage: $0 <aosp-tree> <project>"
    echo "  Example: $0 raspi5-aosp rpi5_custom"
    exit 1
fi

AOSP_DIR="$LOBO_ROOT/$AOSP_TREE"

if [ ! -d "$AOSP_DIR" ]; then
    echo "Error: AOSP tree not found at $AOSP_DIR"
    exit 1
fi

SOURCE_LOBO="$PLATFORM_DIR/vendor/lobo"
SOURCE_PROJECT="$PLATFORM_DIR/projects/$PROJECT"
MOUNT_LOBO="$AOSP_DIR/vendor/lobo"
MOUNT_PROJECT="$AOSP_DIR/vendor/projects/$PROJECT"

echo "Platform dir : $PLATFORM_DIR"
echo "AOSP tree    : $AOSP_DIR"
echo "Project      : $PROJECT"
echo ""

# Create mount point directories
echo "Creating mount point directories..."
mkdir -p "$MOUNT_LOBO"
mkdir -p "$MOUNT_PROJECT"

# Mount vendor/lobo
if mountpoint -q "$MOUNT_LOBO"; then
    echo "vendor/lobo already mounted — skipping"
else
    echo "Mounting vendor/lobo..."
    sudo mount --bind "$SOURCE_LOBO" "$MOUNT_LOBO"
fi

# Mount vendor/projects/<project>
if mountpoint -q "$MOUNT_PROJECT"; then
    echo "vendor/projects/$PROJECT already mounted — skipping"
else
    echo "Mounting vendor/projects/$PROJECT..."
    sudo mount --bind "$SOURCE_PROJECT" "$MOUNT_PROJECT"
fi

echo ""
echo "Done. Verify with:"
echo "  mount | grep lobo"
echo ""
echo "To persist across reboots, add to /etc/fstab:"
echo "  $SOURCE_LOBO  $MOUNT_LOBO  none  bind  0  0"
echo "  $SOURCE_PROJECT  $MOUNT_PROJECT  none  bind  0  0"
