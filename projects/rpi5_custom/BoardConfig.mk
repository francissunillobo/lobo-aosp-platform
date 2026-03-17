# Inherit all board config from upstream rpi5 — do NOT modify device/brcm/rpi5/
-include device/brcm/rpi5/BoardConfig.mk

# Lobo-specific board overrides (add only what differs from upstream)
BOARD_VENDOR_KERNEL_MODULES_LOAD += lobo_fan.ko
