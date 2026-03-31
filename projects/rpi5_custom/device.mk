# Lobo-only additions for rpi5_custom.
# Do NOT $(call inherit-product, device/brcm/rpi5/device.mk) here — that chain is
# already applied by rpi5_custom.mk → aosp_rpi5.mk.

# Packages and file copies for this product
# NOTE: calculatord + CalculatorClientApp; add more Lobo packages here as needed.
PRODUCT_PACKAGES += \
    calculatord \
    CalculatorClientApp \
    ThermalMonitorApp \
    temperature_monitord_rpi5

# Install init rc files via PRODUCT_COPY_FILES to avoid Soong/Make bridge conflict
PRODUCT_COPY_FILES += \
    vendor/lobo/services/calculator/calculator.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/calculator.rc \
    vendor/lobo/services/temperature_monitor/temperature_monitor.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/temperature_monitor.rc \
    init/hw/init.rpi5.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.rpi5.rc
