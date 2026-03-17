LOCAL_PATH := $(call my-dir)

# Register Lobo platform products with AOSP
PRODUCT_MAKEFILES += \
    $(LOCAL_PATH)/../../projects/rpi5_custom/rpi5_custom.mk
