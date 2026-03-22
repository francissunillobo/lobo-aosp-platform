#!/bin/bash
# setup-bind-mounts.sh
# Sets up bind mounts to connect lobo-aosp-platform into an AOSP tree.
#
# Usage:
#   ./scripts/setup-bind-mounts.sh <aosp-tree> <project>
#
# Examples:
#   ./scripts/setup-bind-mounts.sh raspi5-aosp rpi5_custom
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
