# lobo-aosp-platform

Lobo AOSP Platform — custom vendor overlay for Android 16 (rpi5_custom).

## Structure
- `vendor/lobo/` — common libs, services, apps
- `projects/rpi5_custom/` — product config (inherits device/brcm/rpi5/) + HAL

## Integration

This repo lives outside the AOSP tree. It is connected via **bind mounts**:

```bash
sudo mount --bind /root/lobo-aosp/lobo-aosp-platform/vendor/lobo \
                  /root/lobo-aosp/raspi5-aosp/vendor/lobo

sudo mount --bind /root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom \
                  /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom
```

See `docs/PROJECT_GUIDE.md` for full setup instructions.

## Build
```
lunch rpi5_custom-trunk_staging-userdebug
make bootimage systemimage vendorimage -j$(nproc)
```
