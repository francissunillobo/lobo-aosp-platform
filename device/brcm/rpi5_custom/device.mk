# Inherit everything from the upstream rpi5 product
$(call inherit-product, device/brcm/rpi5/device.mk)

PRODUCT_NAME    := rpi5_custom
PRODUCT_DEVICE  := rpi5_custom
PRODUCT_BRAND   := Lobo
PRODUCT_MODEL   := Lobo RPi5
PRODUCT_MANUFACTURER := Lobo

# Add lobo platform packages
PRODUCT_PACKAGES += \
    FanControlService \
    FanSettingsService \
    NameService \
    MySystemApp \
    MyUserApp
