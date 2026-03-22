# Included from vendor/lobo/Android.mk — LOCAL_DIR is set there to this directory.

PRODUCT_MAKEFILES += $(LOCAL_DIR)/rpi5_custom_car.mk

COMMON_LUNCH_CHOICES += \
    rpi5_custom_car-trunk_staging-userdebug \
    rpi5_custom_car-trunk_staging-user \
    rpi5_custom_car-trunk_staging-eng
