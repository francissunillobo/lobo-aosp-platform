# Inherit everything from the upstream rpi5 product
$(call inherit-product, device/brcm/rpi5/device.mk)

# Add lobo platform packages
PRODUCT_PACKAGES += \
    FanControlService \
    FanSettingsService \
    NameService \
    MySystemApp \
    MyUserApp

# Install init rc files via PRODUCT_COPY_FILES to avoid Soong/Make bridge conflict
PRODUCT_COPY_FILES += \
    vendor/lobo_platform/vendor/lobo/services/fancontrol/fancontrol.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/fancontrol.rc \
    vendor/lobo_platform/vendor/lobo/services/name_service/name_service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/name_service.rc
