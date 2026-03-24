# Inherit all board config from upstream rpi5 — do NOT modify device/brcm/rpi5/
-include device/brcm/rpi5/BoardConfig.mk

# Lobo vendor SELinux policy directories.
# Each service under vendor/lobo/services/<name>/sepolicy/ must be listed here
# so the build compiles .te, file_contexts, and service_contexts into the image.
# Without this, init refuses to start vendor services (undefined domain).
BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/services/calculator/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/apps/system/CalculatorClientApp/sepolicy

# Disable Android Verified Boot (AVB).
# RPi5 bootloader does not support AVB. Disabling it allows adb remount to work
# during development so SELinux and vendor files can be pushed without reflashing.
BOARD_AVB_ENABLE := false
