# Inherit the full upstream rpi5 product (includes full_base.mk, overlays, permissions)
$(call inherit-product, device/brcm/rpi5/aosp_rpi5.mk)

# Lobo packages + PRODUCT_COPY_FILES (single place — see device.mk)
# Path is relative to AOSP $TOP: bind mount maps lobo-aosp-platform/projects/rpi5_custom
# → <tree>/vendor/projects/rpi5_custom, so this file is $TOP/vendor/projects/rpi5_custom/device.mk
$(call inherit-product, vendor/projects/rpi5_custom/device.mk)

PRODUCT_NAME         := rpi5_custom
PRODUCT_DEVICE       := rpi5_custom
PRODUCT_BRAND        := Lobo
PRODUCT_MODEL        := Lobo RPi5
PRODUCT_MANUFACTURER := Lobo
