LOCAL_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
# Lobo vendor directory (libs, services, apps under this tree).
#
# Products live in projects/*/AndroidProducts.mk — do NOT include those files here.
# Soong emits their paths to $(OUT_DIR)/.module_paths/AndroidProducts.mk.list;
# build/make/core/product_config.mk (_read-ap-file) sets LOCAL_DIR, clears
# PRODUCT_MAKEFILES / COMMON_LUNCH_CHOICES per file, includes each once, and validates.
# A duplicate include from this Android.mk can break lunch / product registration.
#
# See: docs/PROJECT_GUIDE.md §9
# ---------------------------------------------------------------------------
