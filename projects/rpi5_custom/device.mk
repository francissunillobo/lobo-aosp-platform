# Lobo-only additions for rpi5_custom.
# Do NOT $(call inherit-product, device/brcm/rpi5/device.mk) here — that chain is
# already applied by rpi5_custom.mk → aosp_rpi5.mk.

# Packages and file copies for this product
# NOTE: Only calculatord enabled for initial integration testing.
# FanControlService, FanSettingsService, NameService, MySystemApp, MyUserApp
# are temporarily disabled until calculatord builds and runs successfully.
PRODUCT_PACKAGES += \
    calculatord

# Install init rc files via PRODUCT_COPY_FILES to avoid Soong/Make bridge conflict
PRODUCT_COPY_FILES += \
    vendor/lobo/services/calculator/calculator.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/calculator.rc
