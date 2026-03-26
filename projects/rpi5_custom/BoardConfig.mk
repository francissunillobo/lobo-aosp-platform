# Inherit all board config from upstream rpi5 — do NOT modify device/brcm/rpi5/
-include device/brcm/rpi5/BoardConfig.mk



# Vendor SELinux for calculatord and CalculatorClientApp (same as car product).
BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/services/calculator/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/apps/system/CalculatorClientApp/sepolicy
