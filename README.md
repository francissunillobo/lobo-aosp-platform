# lobo-aosp-platform

Lobo AOSP Platform — custom vendor overlay for Android 15 (rpi5_custom).

## Structure
- `vendor/lobo/` — common libs, services, apps
- `device/brcm/rpi5_custom/` — product config (inherits device/brcm/rpi5/) + HAL

## Repo integration
Add to `manifest/manifest_brcm_rpi.xml` in raspberry-vanilla_android-15.0.0_r14:
```xml
<project name="francissunillobo/lobo-aosp-platform"
         path="."
         remote="github"
         revision="main" />
```

## Build
```
lunch rpi5_custom-userdebug && m
```
