# Inherit the full upstream rpi5 product (includes full_base.mk, overlays, permissions)
$(call inherit-product, device/brcm/rpi5/aosp_rpi5.mk)

PRODUCT_NAME         := rpi5_custom
PRODUCT_DEVICE       := rpi5_custom
PRODUCT_BRAND        := Lobo
PRODUCT_MODEL        := Lobo RPi5
PRODUCT_MANUFACTURER := Lobo

# Lobo platform packages
PRODUCT_PACKAGES += \
    FanControlService \
    FanSettingsService \
    NameService \
    MySystemApp \
    MyUserApp
