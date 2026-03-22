# Inherit the full upstream rpi5 car product (includes car.mk, vehicle HAL, EVS, CAN)
$(call inherit-product, device/brcm/rpi5/aosp_rpi5_car.mk)

# Lobo packages + PRODUCT_COPY_FILES (single place — see device.mk)
$(call inherit-product, vendor/projects/rpi5_custom_car/device.mk)

PRODUCT_NAME         := rpi5_custom_car
PRODUCT_DEVICE       := rpi5_custom_car
PRODUCT_BRAND        := Lobo
PRODUCT_MODEL        := Lobo RPi5 Car
PRODUCT_MANUFACTURER := Lobo
