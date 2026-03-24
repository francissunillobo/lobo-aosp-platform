# Folder Structure Guidelines — lobo-aosp-platform

**What:** The canonical folder layout for every module in `lobo-aosp-platform` —
libraries, services, apps, and device projects. Every new file must follow these
conventions exactly.

**Why:** A consistent structure means any developer can navigate an unfamiliar module
by pattern recognition alone. The AOSP build system (Soong / Make) requires specific
paths to find sources, headers, and AIDL files. Deviations cause silent build failures
or runtime SELinux denials. A shared layout also makes code reviews predictable —
any deviation from structure is immediately visible as a violation.

**How:** Find the section that matches what you are building. Follow the directory
tree exactly. The rules at the bottom of each section are not optional.

---

## Table of Contents

1. [Top-Level Repository Layout](#1-top-level-repository-layout)
2. [C++ Common Libraries](#2-c-common-libraries)
3. [Java / Kotlin Common Libraries](#3-java--kotlin-common-libraries)
4. [C++ Native Services (Binder Daemons)](#4-c-native-services-binder-daemons)
5. [Java / Kotlin Services (Android App Services)](#5-java--kotlin-services-android-app-services)
6. [System Apps](#6-system-apps)
7. [User Apps](#7-user-apps)
8. [Device Projects](#8-device-projects)
9. [Naming Rules](#9-naming-rules)
10. [Universal Rules](#10-universal-rules)

---

## 1. Top-Level Repository Layout

### What

The root of `lobo-aosp-platform`. Every module lives under `vendor/lobo/` or
`projects/`. Documentation lives under `docs/`.

```
lobo-aosp-platform/
├── vendor/lobo/               ← all platform code (libraries, services, apps)
│   ├── Android.mk             ← root makefile — product discovery comments only
│   ├── vendorsetup.sh         ← adds lunch combos
│   ├── common/                ← shared code used by multiple modules
│   │   ├── cpp/               ← shared C++ libraries
│   │   └── java/              ← shared Java/Kotlin library
│   ├── services/              ← background daemons and Android bound services
│   └── apps/
│       ├── system/            ← privileged vendor apps (platform certificate)
│       └── user/              ← regular user-facing apps
├── projects/                  ← one folder per AOSP target device
│   ├── rpi5_custom/
│   └── rpi5_custom_car/
└── docs/                      ← all project documentation
```

### Why

`vendor/lobo/` maps directly to the `vendor/lobo/` path inside every AOSP tree via
**bind mount** (not a symlink). The bind mounts are set up on the build server so
that `lobo-aosp-platform/vendor/lobo` appears at `raspi5-aosp/vendor/lobo` without
copying files. Soong scans this path automatically. Splitting into `common/`,
`services/`, and `apps/` reflects ownership: common code has no runtime dependencies,
services run as daemons, apps have UI.

### How — Setting up bind mounts

`lobo-aosp-platform` lives at `/root/lobo-aosp/lobo-aosp-platform/`, outside every
AOSP tree. Before building, bind mounts must connect it into the AOSP tree. A script
handles this:

```bash
cd /root/lobo-aosp/lobo-aosp-platform

# Mount vendor/lobo + one project into raspi5-aosp
./scripts/setup-bind-mounts.sh raspi5-aosp rpi5_custom
./scripts/setup-bind-mounts.sh raspi5-aosp rpi5_custom_car
```

What the script does for each call:
1. Creates an empty directory at `<aosp-tree>/vendor/lobo` (mount point)
2. Creates an empty directory at `<aosp-tree>/vendor/projects/<project>` (mount point)
3. `mount --bind lobo-aosp-platform/vendor/lobo → <aosp-tree>/vendor/lobo`
4. `mount --bind lobo-aosp-platform/projects/<project> → <aosp-tree>/vendor/projects/<project>`

The script is idempotent — running it again when already mounted skips without error.

**Verify mounts are active:**

```bash
mount | grep lobo
# Expected output (three lines):
# /dev/md2 on /root/lobo-aosp/raspi5-aosp/vendor/lobo ...
# /dev/md2 on /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom ...
# /dev/md2 on /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom_car ...
```

**Persistence across reboots — `/etc/fstab`:**

The bind mounts are added to `/etc/fstab` so they survive a server reboot:

```
/root/lobo-aosp/lobo-aosp-platform/vendor/lobo               /root/lobo-aosp/raspi5-aosp/vendor/lobo                      none bind 0 0
/root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom       /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom      none bind 0 0
/root/lobo-aosp/lobo-aosp-platform/projects/rpi5_custom_car   /root/lobo-aosp/raspi5-aosp/vendor/projects/rpi5_custom_car  none bind 0 0
```

**Adding a new AOSP tree (e.g. vim3-aosp):**

```bash
./scripts/setup-bind-mounts.sh vim3-aosp vim3_custom
```

Then add the corresponding lines to `/etc/fstab` for persistence.

### How — `Android.mk` rule

`Android.mk` must contain only comments. It must **never** `$(eval include ...)` any
`AndroidProducts.mk` file. AOSP's Soong scanner discovers `AndroidProducts.mk` files
independently. A duplicate include resets `PRODUCT_MAKEFILES` and causes:

```
COMMON_LUNCH_CHOICES contains products not defined in this file
```

---

## 2. C++ Common Libraries

### What

Reusable C++ code shared by two or more services or HALs. Compiled as a static
library. Examples: `liblobo_logging`, `liblobo_fan_math`.

```
common/cpp/<library-name>/
├── Android.bp
├── include/
│   └── lobo/
│       └── platform/
│           └── <library-name>/
│               └── MyHeader.h          ← public API headers only
├── src/
│   └── MyImpl.cpp                      ← implementation
└── tests/
    └── src/
        └── MyLibraryTest.cpp
```

### Why

**`include/lobo/platform/<library-name>/` (scoped namespace):**
Without this nesting, two libraries could both have a `Logging.h` and a consumer
including `"Logging.h"` would get whichever the compiler finds first. The full path
`lobo/platform/logging/Logging.h` is unique across the entire AOSP tree.

**`src/` separated from `include/`:**
`Android.bp` lists `srcs: ["src/*.cpp"]`. Implementation files must not go into
`include/` — that directory is exported to consumers and should contain only headers.

**`tests/src/` co-located with the library:**
Tests live inside the module folder, not in a global `tests/` tree. This keeps each
module self-contained — when you delete a module you delete its tests automatically.

### How — Android.bp

```
cc_library_static {
    name: "liblobo_<library-name>",    ← must start with liblobo_
    vendor: true,
    srcs: ["src/*.cpp"],
    export_include_dirs: ["include"],  ← exports the include/ root
    cflags: ["-Wall", "-Werror"],
}
```

`export_include_dirs: ["include"]` exports the `include/` directory root. Consumers
get `lobo/platform/<library-name>/` as their include path automatically. They write:

```cpp
#include "lobo/platform/logging/Logging.h"   // correct
#include "Logging.h"                          // wrong — do not do this
```

---

## 3. Java / Kotlin Common Libraries

### What

Reusable Kotlin/Java code shared across apps and services on the Java side. Compiled
as a `java_library`. Example: `lobo-common-java`.

```
common/java/
├── Android.bp
└── src/
    └── main/
        └── java/
            └── com/
                └── lobo/
                    └── platform/
                        └── <feature>/
                            ├── api/
                            │   └── MyInterface.kt
                            └── impl/
                                └── MyImpl.kt
```

### Why

**`src/main/java/` is required by Soong:**
Soong uses the glob `srcs: ["src/main/java/**/*.kt"]`. If a `.kt` file is placed
outside this path (e.g. directly under `src/`) Soong silently ignores it — the build
succeeds but the class is missing. This is `src/main/java/` is the Maven standard
source layout and AOSP adopted it for Java/Kotlin modules.

**Package path must mirror directory path:**
The Java compiler enforces that a file declaring `package com.lobo.platform.common.logging`
lives at `src/main/java/com/lobo/platform/common/logging/`. A mismatch causes a
compilation error (`package name ... does not match directory`).

**`api/` and `impl/` inside every feature:**

| Folder | Contents | Rule |
|--------|----------|------|
| `api/` | Interfaces, contracts, data classes | No implementation logic. This is what callers import. |
| `impl/` | Concrete implementations | Depends on `api/`. Callers never import from `impl/` directly. |

This split allows test code to implement the `api/` interface (mock) without touching
`impl/`, and allows swapping implementations without breaking callers.

### How — Android.bp

```
java_library {
    name: "lobo-common-java",          ← prefix: lobo- with hyphen (Java convention)
    vendor: true,
    srcs: ["src/main/java/**/*.kt"],
    sdk_version: "current",
}
```

---

## 4. C++ Native Services (Binder Daemons)

### What

A background process started by `init` at boot, exposing operations over Android
Binder IPC. Examples: `calculatord`, `FanControlService`, `NameService`.

```
services/<service-name>/
├── Android.bp
├── <service-name>.rc                       ← init script (starts the daemon at boot)
├── aidl/
│   └── com/
│       └── lobo/
│           └── platform/
│               └── <service>/
│                   └── IMyService.aidl     ← IPC contract (single source of truth)
├── cpp/
│   ├── core/
│   │   ├── include/
│   │   │   └── lobo/platform/<service>/
│   │   │       ├── IMyService.h            ← abstract C++ interface (mockable)
│   │   │       └── MyServiceImpl.h
│   │   └── src/
│   │       ├── MyServiceImpl.cpp           ← logic
│   │       └── main.cpp                    ← daemon entry point
│   └── client/
│       ├── include/
│       │   └── lobo/platform/<service>/
│       │       └── MyServiceClient.h
│       └── src/
│           └── MyServiceClient.cpp         ← lazy Binder connection wrapper
├── java/
│   └── src/
│       └── main/
│           └── java/
│               └── com/lobo/platform/<service>/
│                   ├── api/
│                   │   └── IMyServiceClient.kt
│                   └── impl/
│                       └── MyServiceClientImpl.kt
├── sepolicy/
│   ├── <service>.te
│   ├── service.te
│   ├── file_contexts
│   └── service_contexts
└── tests/
    └── src/
        └── VtsMyServiceTest.cpp
```

### Why

**`aidl/com/lobo/platform/<service>/`:**
The AIDL compiler requires the directory path to match the Java package name exactly.
`IMyService.aidl` with `package com.lobo.platform.myservice` must live at
`aidl/com/lobo/platform/myservice/`. A mismatch is a build error.

**`cpp/core/` vs `cpp/client/`:**
The daemon (core) and the client library are separate Soong build targets. Other
services that want to call this service link only `liblobo_<service>_client` — they
never link the daemon binary. This separation enforces the client/server boundary at
the build level.

**`java/src/main/java/`:**
The Kotlin client library for Android apps. It lives inside the service folder because
it is part of the service definition (same AIDL, same package). It is NOT an app —
it is a library. It follows the same `src/main/java/` rule as Section 3.

**`sepolicy/` — all four files are mandatory:**

| File | What | Why it is required |
|------|------|--------------------|
| `<service>.te` | Process domain — Binder, logging, registration permissions | Without it, SELinux denies every action the daemon takes at runtime |
| `service.te` | Service type in ServiceManager | Without it, clients cannot look up the service by name |
| `file_contexts` | Labels `/vendor/bin/<service>d` as `<service>d_exec` | Without it, `init` runs the binary in the wrong domain; process can't do anything |
| `service_contexts` | Labels the ServiceManager entry | Without it, Binder IPC from clients is blocked |

The `sepolicy/` path must be added to `BoardConfig.mk`:
```makefile
BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/services/<service>/sepolicy
```

Without this line, the four files are ignored by the build — they are not compiled
into the image.

**`tests/src/VtsMyServiceTest.cpp`:**
VTS tests run on the real device against the live Binder service. Using a mock defeats
the purpose — the test must verify the real binary on real hardware.

### How — Android.bp structure

```
aidl_interface { name: "lobo_<service>_aidl" }      ← NDK + Java stubs
cc_binary      { name: "<service>d" }                ← daemon binary
cc_library_static { name: "liblobo_<service>_client" } ← C++ client lib
java_library   { name: "Lobo<Service>JavaClient" }   ← Kotlin client lib
cc_test        { name: "Vts<Service>Test" }           ← VTS tests
```

---

## 5. Java / Kotlin Services (Android App Services)

### What

A service implemented as an Android `Service` component (bound or started), packaged
as an APK. Example: `FanSettingsService`.

```
services/<service-name>/
├── Android.bp                              ← android_app module
├── AndroidManifest.xml                     ← declares Service component
├── <service-name>.rc                       ← init script (if started at boot)
├── aidl/
│   └── com/lobo/platform/<service>/
│       └── IMyService.aidl
├── res/
│   └── values/
│       └── strings.xml
├── src/
│   └── main/
│       └── java/
│           └── com/lobo/platform/<service>/
│               ├── api/
│               │   └── MyServiceContract.kt
│               └── impl/
│                   ├── MyService.kt
│                   └── MyServiceManager.kt
└── sepolicy/
    └── <service>.te
```

### Why

Java services differ from C++ services in one key way: the entire service is
Java/Kotlin, so there is no `cpp/` or `java/` sub-folder split. Sources live directly
under `src/main/java/`. The `api/` and `impl/` split still applies.

### How

```
android_app {
    name: "<ServiceName>",
    vendor: true,
    srcs: ["src/main/java/**/*.kt"],
    resource_dirs: ["res"],
    manifest: "AndroidManifest.xml",
    sdk_version: "current",
    certificate: "platform",
}
```

---

## 6. System Apps

### What

An Android app that needs elevated permissions, communicates with vendor services, or
runs on the vendor partition. Signed with the platform certificate. Example:
`MySystemApp`.

```
apps/system/<AppName>/
├── Android.bp
├── AndroidManifest.xml
├── res/
│   └── values/
│       └── strings.xml
├── sepolicy/
│   ├── <appname>.te                        ← custom domain + permissions
│   └── seapp_contexts                      ← maps package name to the domain
└── src/
    └── main/
        └── java/
            └── com/lobo/platform/<appname>/
                ├── di/                     ← dependency injection modules
                ├── <feature>/
                │   ├── api/                ← interface / contract
                │   └── impl/               ← implementation
                └── ...                     ← one sub-folder per feature
```

### Why

**`vendor: true`:**
Places the APK on the vendor partition (`/vendor/priv-app/`). SELinux only allows
vendor-domain processes to call vendor Binder services. A system-partition app
(`vendor: false`) cannot reach a service registered in the vendor ServiceManager.

**`privileged: true`:**
Grants `signature|privileged` permissions declared in the framework. Without this,
calls to protected system APIs fail silently at runtime with a security exception.

**`certificate: "platform"`:**
Platform certificate is required for apps that use `android:sharedUserId="android.uid.system"`
or access APIs restricted to platform-signed apps.

**`sepolicy/` — required for all Lobo vendor system apps:**
Vendor privileged apps that call custom Binder services, access vendor resources, or
use vendor HALs need an explicit SELinux domain. Without it the app relies on the
generic `platform_app` domain which may be denied at runtime with no clear log trail.
Start minimal and extend only when `avc: denied` appears in logcat.

Two files are required (different from native daemons — no `file_contexts` or `service_contexts`):

| File | What | Why |
|------|------|-----|
| `<appname>.te` | Domain definition and permission rules | Defines what the app is allowed to do. `app_domain()` grants base app permissions. |
| `seapp_contexts` | Maps the package name to the domain | Without this the app runs in the generic `platform_app` domain instead of the custom one. |

**Important — wrong pattern to avoid:**
Do NOT add `exec_type` or `vendor_file_type` to an app's `.te` file. Those are for
native binaries launched by `init`. An Android app is spawned by Zygote, not exec'd
from disk.

```te
# WRONG — exec_type is for native daemons, not apps
type mysystemapp_exec, exec_type, vendor_file_type, file_type;   ← remove this

# CORRECT minimal pattern for a vendor privileged app
type mysystemapp, domain;
app_domain(mysystemapp)
binder_use(mysystemapp)
```

The `sepolicy/` path must also be registered in `BoardConfig.mk`:
```makefile
BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/apps/system/<AppName>/sepolicy
```

**`di/` folder:**
Dependency injection wiring (e.g. Hilt modules, manual DI). Kept separate from
features so that feature code does not know about how dependencies are assembled.

**`<feature>/api/` and `<feature>/impl/`:**
Same rule as every other layer — contracts in `api/`, implementations in `impl/`.
Apps interact with their own features through the `api/` interface. This makes unit
testing possible without starting an Activity.

### How — Android.bp

```
android_app {
    name: "<AppName>",
    vendor: true,
    privileged: true,
    srcs: ["src/main/java/**/*.kt"],
    resource_dirs: ["res"],
    manifest: "AndroidManifest.xml",
    sdk_version: "current",
    certificate: "platform",
    static_libs: ["lobo-common-java"],
}
```

---

## 7. User Apps

### What

A regular Android app visible to the end user. No elevated permissions, not on the
vendor partition. Example: `MyUserApp`.

```
apps/user/<AppName>/
├── Android.bp
├── AndroidManifest.xml
├── res/
│   └── values/
│       └── strings.xml
└── src/
    └── main/
        └── java/
            └── com/lobo/platform/<appname>/
                ├── di/
                ├── <feature>/
                │   ├── api/
                │   └── impl/
                └── ...
```

### Why

User apps follow the same internal structure as system apps. The only differences are
in `Android.bp`: no `vendor: true`, no `privileged: true`, no `certificate: "platform"`.
No `sepolicy/` unless the app requires an explicit custom SELinux rule.

### How — Android.bp

```
android_app {
    name: "<AppName>",
    srcs: ["src/main/java/**/*.kt"],
    resource_dirs: ["res"],
    manifest: "AndroidManifest.xml",
    sdk_version: "current",
    static_libs: ["lobo-common-java"],
}
```

---

## 8. Device Projects

### What

Device-specific configuration — product definition, board settings, HAL
implementations. One folder per AOSP target. Example: `rpi5_custom_car`.

```
projects/<target-name>/
├── AndroidProducts.mk              ← PRODUCT_MAKEFILES + COMMON_LUNCH_CHOICES (same file)
├── <target-name>.mk                ← product makefile (inherits upstream device mk)
├── BoardConfig.mk                  ← board-level overrides
├── device.mk                       ← packages, properties, copy-files
└── hal/
    ├── interfaces/
    │   ├── Android.bp              ← cc_library_headers module
    │   └── include/
    │       └── lobo/platform/hal/
    │           ├── IFanHal.h       ← abstract HAL interface
    │           └── IThermalHal.h
    └── <hal-type>/
        ├── Android.bp
        ├── include/
        │   └── lobo/platform/hal/<target>/
        │       └── MyHalImpl.h
        └── src/
            └── MyHalImpl.cpp
```

### Why

**`AndroidProducts.mk` — both variables in the same file:**
AOSP's `_read-ap-file` macro in `product_config.mk` resets `PRODUCT_MAKEFILES` and
`COMMON_LUNCH_CHOICES` before each include. Both must be set in the same
`AndroidProducts.mk`. Setting one in a parent file and the other here will produce
a validation error on every `lunch`.

**`BoardConfig.mk` — overrides only, never upstream changes:**
This file uses `-include device/brcm/rpi5/BoardConfig.mk` then adds Lobo-specific
overrides. Changes to the upstream file (`device/brcm/rpi5/`) are forbidden — that
file is owned by the community and overwritten on `repo sync`.

**`hal/interfaces/` separated from `hal/<type>/`:**
The abstract interfaces (`IFanHal.h`) are shared across all HAL implementations.
Keeping them in `interfaces/` means a new device target can provide a different
implementation without touching the interface.

### How — `AndroidProducts.mk` required format

```makefile
LOCAL_DIR := $(call my-dir)
PRODUCT_MAKEFILES += $(LOCAL_DIR)/<target>.mk
COMMON_LUNCH_CHOICES += \
    <target>-trunk_staging-userdebug \
    <target>-trunk_staging-user \
    <target>-trunk_staging-eng
```

---

## 9. Naming Rules

| Layer | C++ module name | Java module name | Package |
|-------|----------------|------------------|---------|
| Common C++ lib | `liblobo_<name>` | — | — |
| Common Java lib | — | `lobo-common-java` | `com.lobo.platform.common.<name>` |
| Service AIDL | `lobo_<service>_aidl` | — | `com.lobo.platform.<service>` |
| Service daemon | `<service>d` | — | — |
| Service C++ client | `liblobo_<service>_client` | — | — |
| Service Kotlin client | — | `Lobo<Service>JavaClient` | `com.lobo.platform.<service>` |
| System app | — | `<AppName>` | `com.lobo.platform.<appname>` |
| User app | — | `<AppName>` | `com.lobo.platform.<appname>` |
| HAL interfaces | `liblobo_hal_interfaces_headers` | — | — |
| HAL implementation | `liblobo_hal_<target>` | — | — |

**C++:** `liblobo_` prefix, underscores between words.
**Java/Kotlin:** `lobo-` prefix or PascalCase app name, hyphens between words.

---

## 10. Universal Rules

These rules apply to every module regardless of type.

### What

Eight rules that every module must follow.

### Why and How

**1. `api/` is for contracts, `impl/` is for implementations.**
- Why: Callers that import `api/` are isolated from implementation changes. Mocking in tests requires only an `api/` implementation.
- How: Create `api/` with interfaces and `impl/` with concrete classes. Never let a caller import a class from `impl/`.

**2. C++ headers always live under `include/lobo/platform/<subsystem>/`.**
- Why: Scoped paths prevent name collisions across modules. AOSP has thousands of headers — a flat path guarantees conflicts.
- How: Include as `#include "lobo/platform/logging/Logging.h"`. Never as `#include "Logging.h"`.

**3. Java/Kotlin sources always live under `src/main/java/`.**
- Why: Soong's glob `srcs: ["src/main/java/**/*.kt"]` will silently skip files outside this path.
- How: Verify with `find <module>/ -name "*.kt" | grep -v src/main/java` — output must be empty.

**4. The AIDL directory path must match the Java package name.**
- Why: The AIDL compiler derives the package from the file path. A mismatch is a build error.
- How: `package com.lobo.platform.calculator` → file at `aidl/com/lobo/platform/calculator/IMyService.aidl`.

**5. Every C++ native service must have all four `sepolicy/` files.**
- Why: Any single missing file causes either a boot failure or a runtime Binder denial.
- How: After adding files, add the path to `BoardConfig.mk`:
  ```makefile
  BOARD_VENDOR_SEPOLICY_DIRS += vendor/lobo/services/<service>/sepolicy
  ```

**6. Tests are co-located with the module.**
- Why: Deleting a module must also delete its tests. A global `tests/` tree breaks this.
- How: Tests live in `tests/src/` inside the module folder, not elsewhere.

**7. Never modify upstream AOSP or community device files.**
- Why: `repo sync` overwrites them. All overrides go in `projects/<target>/`.
- How: Check that your edit is inside `lobo-aosp-platform/`, never inside `device/brcm/rpi5/`.

**8. Kotlin client libraries for C++ services live in `services/<service>/java/`.**
- Why: The Kotlin client is part of the service definition — it implements the same AIDL contract. It is a library, not an app.
- How: Add the Kotlin files under `services/<service>/java/src/main/java/`. Only promote to `apps/system/` if the module becomes a standalone application with its own UI.

---

**Author:** Francis Lobo · **Project:** lobo-aosp-platform
