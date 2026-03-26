# Calculator Platform Guide (Service + Client App)

**What:** This document explains the complete calculator stack in `lobo-aosp-platform`: the vendor Binder service (`calculatord`), the AIDL contract, and the `CalculatorClientApp` UI app.

**Why:** The calculator modules are the reference pattern for future Lobo services/apps. This guide records the final architecture, integration choices, known AIDL pitfalls, and device verification commands.

**How:** Use this guide when developing, debugging, or onboarding. It includes folder maps, runtime flow, lessons learned, and ready-to-run `adb` checks.

For broader platform context, also refer to:
- `docs/FOLDER_STRUCTURE_GUIDELINES.md`
- `docs/PROJECT_GUIDE.md`

---

## 1) Project Folder Map

### What
The calculator feature is split into two main modules plus product wiring.

### Why
Keeping service and app separated mirrors production Android layering:
vendor daemon in `vendor/lobo/services/...` and client UI in `vendor/lobo/apps/...`.

### How
Use this map as the source of truth for where each responsibility lives.

```
lobo-aosp-platform/
├── vendor/lobo/services/calculator/
│   ├── Android.bp
│   ├── calculator.rc
│   ├── aidl/com/lobo/platform/calculator/ICalculatorService.aidl
│   ├── cpp/
│   │   ├── core/
│   │   │   ├── include/lobo/platform/calculator/
│   │   │   │   ├── ICalculator.h
│   │   │   │   └── CalculatorServiceImpl.h
│   │   │   └── src/
│   │   │       ├── CalculatorServiceImpl.cpp
│   │   │       └── main.cpp
│   │   ├── client/
│   │   │   ├── include/lobo/platform/calculator/CalculatorClient.h
│   │   │   └── src/CalculatorClient.cpp
│   │   ├── java/src/main/java/com/lobo/platform/calculator/
│   │   │   ├── api/   (currently empty)
│   │   │   └── impl/  (currently empty)
│   │   ├── sepolicy/
│   │   │   ├── calculator.te
│   │   │   ├── service.te
│   │   │   ├── file_contexts
│   │   │   └── service_contexts
│   │   └── tests/src/VtsCalculatorServiceTest.cpp
│   │
│   └── apps/system/CalculatorClientApp/
│       ├── Android.bp
│       ├── AndroidManifest.xml
│       ├── res/
│       │   ├── layout/activity_calculator_client.xml
│       │   ├── values/strings.xml
│       │   └── drawable/calculator_edittext_bg.xml
│       ├── sepolicy/
│       │   ├── calculatorclientapp.te
│       │   └── seapp_contexts
│       └── src/main/java/com/lobo/platform/calculator/
│           ├── api/ICalculatorClient.kt
│           ├── impl/CalculatorClientImpl.kt
│           ├── di/AppModule.kt
│           └── client/ui/MainActivity.kt
│
└── projects/rpi5_custom_car/
    ├── device.mk
    └── init/hw/init.rpi5.rc
```

---

## 2) Architecture and Runtime Flow

### What
`calculatord` is an NDK Binder service registered as:
`com.lobo.platform.calculator.ICalculatorService`.
`CalculatorClientApp` is a vendor privileged app that binds to it and exposes UI buttons.

### Why
This pattern demonstrates a full Android vendor service lifecycle:
AIDL contract -> generated stubs -> daemon registration -> client calls -> SELinux enforcement.

### How
Boot and call flow:

```
Boot
 ├─ init imports /vendor/etc/init/calculator.rc
 ├─ starts /vendor/bin/calculatord
 └─ calculatord registers:
      com.lobo.platform.calculator.ICalculatorService

App call (Add/Subtract/Multiply/Divide)
 ├─ MainActivity -> ICalculatorClient
 ├─ CalculatorClientImpl gets binder from ServiceManager.checkService()
 ├─ ICalculatorService.Stub.asInterface(...)
 └─ Binder IPC -> CalculatorServiceImpl -> result back to app
```

---

## 3) AIDL for Beginners (Stub, Proxy, Generated Code)

### What
AIDL (Android Interface Definition Language) is the contract language used by Binder IPC. In this project, the file is:
`vendor/lobo/services/calculator/aidl/com/lobo/platform/calculator/ICalculatorService.aidl`

### Why
Beginners often see `Stub`, `Proxy`, `Bn...`, `Bp...`, and `asInterface(...)` and it feels magical. The key idea is:
- you write one `.aidl` interface
- Android generates server/client glue code
- your service implements the server side
- your app uses the client side

### How

#### 3.1 Create the AIDL contract
Write method signatures only (no business logic) in `ICalculatorService.aidl`:

```aidl
interface ICalculatorService {
    int add(int a, int b);
    int subtract(int a, int b);
    int multiply(int a, int b);
    int divide(int a, int b);
    const int ERROR_DIVIDE_BY_ZERO = 1;
}
```

Rules:
- Package name in file must match path.
- Keep AIDL as single source of truth for cross-process API.
- Add error codes/constants here if clients need them.

#### 3.2 What the build generates from AIDL
From `aidl_interface "lobo_calculator_aidl"`, Soong generates:

- **Java/Kotlin side**
  - `ICalculatorService.Stub` (server-side binder adapter class)
  - `ICalculatorService.Stub.Proxy` (client-side object that marshals calls into Binder transactions)
- **NDK/C++ side**
  - `BnCalculatorService` (server-side base class to inherit)
  - `BpCalculatorService` / interface proxy (client-side binder proxy)

Conceptually:
- **Stub / Bn** = receives parcels from Binder, calls your real implementation.
- **Proxy / Bp** = caller-facing object; when you call `add(2,3)`, it writes parcel + transacts.

#### 3.3 How service side uses Stub/Bn
In C++ service:
- `CalculatorServiceImpl` inherits generated `BnCalculatorService`.
- You implement real methods (`add`, `divide`, etc.).
- In `main.cpp`, you create the implementation and register it with `AServiceManager_addService`.

This is server binding:
1. Binder receives transaction code.
2. Generated `BnCalculatorService` decodes args.
3. Calls your `CalculatorServiceImpl::add(...)`.
4. Encodes return value and sends reply.

#### 3.4 How app/client side uses Proxy
In app:
- Get raw binder handle from `ServiceManager.checkService(serviceName)`.
- Convert binder to typed interface with `ICalculatorService.Stub.asInterface(binder)`.
- Returned object is local implementation (if same process) or `Proxy` (if remote process).

When app calls:
`service.add(2, 3)`

under the hood:
1. Proxy writes interface token + args into `Parcel`.
2. Proxy calls `transact(...)`.
3. Remote service executes implementation.
4. Reply parcel is read and converted back to return value.

#### 3.5 Simple mental model
- `.aidl` = API contract.
- `Stub/Bn` = "receiver/decoder" side.
- `Proxy/Bp` = "sender/encoder" side.
- Your class (`CalculatorServiceImpl`) = business logic.

#### 3.6 Adding a new AIDL method (beginner checklist)
1. Add method to `ICalculatorService.aidl`.
2. Implement in `CalculatorServiceImpl.h/.cpp`.
3. Update C++ client wrapper (`CalculatorClient.h/.cpp`) if used.
4. Update app-side client interface (`ICalculatorClient.kt`) and implementation.
5. Rebuild modules.
6. Test from UI and from VTS.

---

## 4) Build and Product Wiring

### What
The feature requires both module build rules and product copy/import wiring.

### Why
If either side is missing, common failures occur:
- daemon binary built but never started (`.rc` not copied/imported)
- app built but service not reachable
- policy present but wrong service labeling

### How

#### Service and AIDL modules
- File: `vendor/lobo/services/calculator/Android.bp`
- Key modules:
  - `aidl_interface "lobo_calculator_aidl"` (NDK + Java backend)
  - `cc_binary "calculatord"` (vendor daemon)
  - `cc_library_static "liblobo_calculator_client"`
  - `cc_test "VtsCalculatorServiceTest"`

#### App module
- File: `vendor/lobo/apps/system/CalculatorClientApp/Android.bp`
- Important flags:
  - `vendor: true`
  - `privileged: true`
  - `sdk_version: "current"` (required for vendor app build rules)

#### Init wiring (car target)
- `projects/rpi5_custom_car/device.mk` must copy:
  - `calculator.rc` -> `/vendor/etc/init/calculator.rc`
  - `init/hw/init.rpi5.rc` -> `/vendor/etc/init/hw/init.rpi5.rc`
- `projects/rpi5_custom_car/init/hw/init.rpi5.rc` must import:
  - `import /vendor/etc/init/calculator.rc`

---

## 5) SELinux Model

### What
SELinux labels both the daemon process and the service_manager entry.

### Why
Without correct types/attributes, either:
- `calculatord` fails to register, or
- app cannot `find`/call the service due to `neverallow`.

### How
- `calculator.te` defines daemon domain and service_manager registration permissions.
- `service_contexts` maps service name to `calculatord_service`.
- `service.te` defines `calculatord_service` with required attributes, notably `app_api_service`.
- `calculatorclientapp.te` grants `find` on `calculatord_service` for the app domain.

---

## 6) AIDL Integration Issues and Final Decisions

### What
These are the key issues faced during calculator AIDL integration and how they were resolved.

### Why
Documenting them avoids repeating hard-to-debug Soong/Binder/SELinux failures.

### How

1. **`platform_apis` + `sdk_version` conflict in app**
   - Issue: Soong rejects both set together.
   - Decision: keep `sdk_version: "current"` and remove `platform_apis`.
   - Reason: vendor/product partition interface enforcement requires SDK usage.

2. **Java AIDL backend API mismatch**
   - Issue: generated Java AIDL defaulted to `system_current`, app compiled with `current`.
   - Decision: set Java backend `sdk_version: "current"` in `aidl_interface`.
   - Reason: consistent API surface between generated stubs and app code.

3. **Hidden API usage (`ServiceManager`)**
   - Issue: direct hidden API imports are not stable for vendor app compile surface.
   - Decision: `CalculatorClientImpl` uses reflection for `ServiceManager.checkService`.
   - Reason: compile-safe access while still using system service lookup.

4. **Binder stability mismatch (`vendor` vs `system`)**
   - Issue seen in logcat:
     - `Cannot do a user transaction on a vendor stability binder ... in a system stability context`
   - Decision: wrap `IBinder` in app and OR `FLAG_PRIVATE_VENDOR (0x10000000)` on transact.
   - Reason: app side uses Java/system Binder context while service is vendor-stability NDK Binder.

5. **SELinux `neverallow` around service type attributes**
   - Issue: removing `app_api_service` from service type breaks app `find`.
   - Decision: keep `calculatord_service` compatible with app discovery by including `app_api_service`.
   - Reason: aligns with platform policy rules for app-visible binder services.

---

## 7) `adb` Verification and Debugging Commands

### What
Commands to verify registration, process health, and runtime calls.

### Why
Calculator failures can come from different layers (init, SELinux, Binder, app UI). These commands isolate each layer quickly.

### How

#### Service/process state
```bash
adb shell ps -A | grep -i calculatord
adb shell pidof calculatord
adb shell service list | grep -i ICalculatorService
adb shell pidof com.lobo.platform.calculator.client
```
Verify: `calculatord` PID exists, app PID exists when app is open, and service list shows `com.lobo.platform.calculator.ICalculatorService`.

#### SELinux domains (portable grep, no ripgrep dependency)
```bash
adb shell ps -Z | grep -Ei "calculatorclientapp|calculatord"
```
Verify: output shows expected domains for `calculatord` and `calculatorclientapp` (not `u:r:untrusted_app` for the privileged app target).

#### Fresh runtime logs for a test session
```bash
adb logcat -c
adb shell am start -W -n com.lobo.platform.calculator.client/.ui.MainActivity
# perform Add/Subtract/etc in UI
adb logcat -d | grep -Ei "Cannot do a user transaction|ICalculatorService|CalculatorClientImpl|service unavailable|calculatord"
```
Verify: no `Cannot do a user transaction ... vendor stability ... system stability` line, and no `service unavailable` errors during button actions.

#### Optional binder call smoke test via app launch + service visibility
```bash
adb shell service check com.lobo.platform.calculator.ICalculatorService
```
Verify: result should not be `not found`.

#### Common interpretation
- `service list` entry present + `calculatord` pid present = daemon started and registered.
- `Cannot do a user transaction ... vendor stability ... system stability` = binder stability mismatch path.
- `service unavailable` from app logs = lookup/proxy unavailable at app side.

---

## 8) Build and Flash/Deploy Quick Reference

### What
Minimal command set to rebuild calculator modules and validate on device.

### Why
Fast iteration reduces time spent rebuilding full images.

### How
On build server:
```bash
cd /root/lobo-aosp/raspi5-aosp
source build/envsetup.sh
lunch rpi5_custom_car-trunk_staging-userdebug

# Module-only iteration
m CalculatorClientApp calculatord -j$(nproc)

# If policy/init/product wiring changed, rebuild vendor image
make vendorimage -j$(nproc)
```
Verify: build ends with `build completed successfully`, and outputs are present under `out/target/product/rpi5_custom_car/vendor/`.

Then flash/deploy using your normal device flow for this target.

---

## 9) UI Notes (Car Display)

### What
The client app UI is tuned for car displays with top and bottom bars.

### Why
AAOS launcher/system bars reduce usable vertical space and can clip labels.

### How
- Theme uses no action bar.
- Buttons use compact heights.
- Layout keeps enough bottom padding to avoid nav bar overlap.
- If a device skin still clips top content, tune top padding in
  `activity_calculator_client.xml`.

---

**Author:** Francis Lobo · **Project:** lobo-aosp-platform
