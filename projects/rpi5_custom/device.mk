# Lobo-only additions for rpi5_custom.
# Do NOT $(call inherit-product, device/brcm/rpi5/device.mk) here — that chain is
# already applied by rpi5_custom.mk → aosp_rpi5.mk.

# Packages and file copies for this product
PRODUCT_PACKAGES += \
    FanControlService \
    FanSettingsService \
    NameService \
    MySystemApp \
    MyUserApp

# Install init rc files via PRODUCT_COPY_FILES to avoid Soong/Make bridge conflict
PRODUCT_COPY_FILES += \
    vendor/lobo/services/fancontrol/fancontrol.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/fancontrol.rc \
    vendor/lobo/services/name_service/name_service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/name_service.rc
