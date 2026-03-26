# Lobo-only additions for rpi5_custom_car.
# Do NOT $(call inherit-product, device/brcm/rpi5/device.mk) here — that chain is
# already applied by rpi5_custom_car.mk → aosp_rpi5_car.mk.

# Packages
# NOTE: Only calculatord enabled for initial integration testing.
PRODUCT_PACKAGES += \
    calculatord \
    CalculatorClientApp

# Install init rc files via PRODUCT_COPY_FILES to avoid Soong/Make bridge conflict
PRODUCT_COPY_FILES += \
    vendor/lobo/services/calculator/calculator.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/calculator.rc

# Ensure init loads our calculator service.
# Upstream vendor init only imports init.rpi5.usb.rc; we override init.rpi5.rc
# to also import /vendor/etc/init/calculator.rc.
PRODUCT_COPY_FILES += \
    init/hw/init.rpi5.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.rpi5.rc
