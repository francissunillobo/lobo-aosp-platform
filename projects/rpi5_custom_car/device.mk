# Lobo-only additions for rpi5_custom_car (AAOS / Raspberry Pi 5 car product).
# Do NOT $(call inherit-product, device/brcm/rpi5/device.mk) here — that chain is
# already applied by rpi5_custom_car.mk → aosp_rpi5_car.mk.

# Vendor packages shipped on this product (bind-mounted lobo-aosp-platform).
PRODUCT_PACKAGES += \
    calculatord \
    CalculatorClientApp \
    ThermalMonitorApp \
    temperature_monitord_rpi5

# Install init rc files via PRODUCT_COPY_FILES to avoid Soong/Make bridge conflict
PRODUCT_COPY_FILES += \
    vendor/lobo/services/calculator/calculator.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/calculator.rc \
    vendor/lobo/services/temperature_monitor/temperature_monitor.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/temperature_monitor.rc

# Override vendor init.rpi5.rc so init imports our service stubs (upstream often
# only pulls init.rpi5.usb.rc). See init/hw/init.rpi5.rc for imports.
PRODUCT_COPY_FILES += \
    init/hw/init.rpi5.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.rpi5.rc
