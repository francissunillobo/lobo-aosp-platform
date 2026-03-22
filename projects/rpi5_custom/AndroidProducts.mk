# Included from vendor/lobo/Android.mk — LOCAL_DIR is set there to this directory.
# PRODUCT_MAKEFILES + COMMON_LUNCH_CHOICES must stay in the same file (AOSP lunch check).

PRODUCT_MAKEFILES += $(LOCAL_DIR)/rpi5_custom.mk

COMMON_LUNCH_CHOICES += \
    rpi5_custom-trunk_staging-userdebug \
    rpi5_custom-trunk_staging-user \
    rpi5_custom-trunk_staging-eng
