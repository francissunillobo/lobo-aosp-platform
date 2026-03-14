# Inherit everything from the upstream rpi5 product
$(call inherit-product, device/brcm/rpi5/device.mk)

# Add lobo platform packages
PRODUCT_PACKAGES += \
    FanControlService \
    FanSettingsService \
    NameService \
    MySystemApp \
    MyUserApp
