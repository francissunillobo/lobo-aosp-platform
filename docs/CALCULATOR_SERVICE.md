# Calculator Service — `vendor/lobo/services/calculator/`

**What:** A vendor Binder service (`calculatord`) that exposes add, subtract, multiply,
and divide operations over IPC. It is the reference example for how Lobo platform
services are structured: AIDL interface → C++ daemon → C++ client library → Kotlin
client library → VTS tests → SELinux policy.

**Why:** Demonstrates the full service lifecycle in one self-contained module so every
new service can follow the same pattern without guessing.

**How:** Read this document top-to-bottom; the sections follow the data/control flow
from build definition → runtime startup → client call.

---

## Directory Layout

```
services/calculator/
├── Android.bp                          ← build rules for all modules
├── calculator.rc                       ← init script (starts the daemon at boot)
├── aidl/
│   └── com/lobo/platform/calculator/
│       └── ICalculatorService.aidl     ← the contract (single source of truth)
├── cpp/
│   ├── core/
│   │   ├── include/…/ICalculator.h              ← abstract C++ interface
│   │   ├── include/…/CalculatorServiceImpl.h    ← Binder impl declaration
│   │   └── src/
│   │       ├── CalculatorServiceImpl.cpp        ← logic (add/sub/mul/div)
│   │       └── main.cpp                         ← daemon entry point
│   └── client/
│       ├── include/…/CalculatorClient.h         ← C++ client API
│       └── src/CalculatorClient.cpp             ← lazy Binder connection
├── java/
│   └── …/calculator/
│       ├── api/ICalculatorClient.kt             ← Kotlin interface
│       └── impl/CalculatorClientImpl.kt         ← Kotlin Binder client
├── sepolicy/
│   ├── calculator.te                   ← SELinux process domain
│   ├── service.te                      ← SELinux service type
│   ├── file_contexts                   ← label for /vendor/bin/calculatord
│   └── service_contexts                ← label for the ServiceManager entry
└── tests/
    └── src/VtsCalculatorServiceTest.cpp ← VTS tests (concurrency, error codes)
```

---

## How the Files Link Together

```
ICalculatorService.aidl  (the contract)
        │
        │  AIDL compiler generates (at build time):
        ├──► BnCalculatorService   (C++ server stub, NDK backend)
        ├──► ICalculatorService    (C++ client proxy, NDK backend)
        └──► ICalculatorService    (Java/Kotlin stub + proxy)

BnCalculatorService
        │  CalculatorServiceImpl inherits from it
        ▼
CalculatorServiceImpl.cpp  (implements add/subtract/multiply/divide)
        │  instantiated in
        ▼
main.cpp  (daemon entry point)
        │  registers via AServiceManager_addService()
        ▼
ServiceManager  (system service registry)
        │
        ├──► CalculatorClient.cpp  (C++ client — calls AServiceManager_checkService)
        │
        └──► CalculatorClientImpl.kt  (Kotlin client — calls ServiceManager.checkService)
```

---

## File-by-File Explanation

### `ICalculatorService.aidl` — The Contract

**What:** Defines the IPC interface. Four methods: `add`, `subtract`, `multiply`,
`divide`. One constant: `ERROR_DIVIDE_BY_ZERO = 1`.

**Why:** AIDL is the single source of truth. The build system generates all C++ and
Java/Kotlin stubs from this one file. No hand-written IPC code.

**How:** Placed under `aidl/com/lobo/platform/calculator/` — the directory path must
match the Java package name `com.lobo.platform.calculator`.

```aidl
interface ICalculatorService {
    int add(int a, int b);
    int subtract(int a, int b);
    int multiply(int a, int b);
    int divide(int a, int b);          // throws ServiceSpecificException on b==0
    const int ERROR_DIVIDE_BY_ZERO = 1;
}
```

---

### `Android.bp` — Build Rules

**What:** Five build targets in one file.

**Why:** Keeps all build rules co-located with the source.

**How:** Each target and what it produces:

| Target | Type | Output | Used by |
|--------|------|--------|---------|
| `lobo_calculator_aidl` | `aidl_interface` | C++ NDK stubs + Java stubs | everything below |
| `calculatord` | `cc_binary` | `/vendor/bin/calculatord` | init (started at boot) |
| `liblobo_calculator_client` | `cc_library_static` | `.a` static lib | other C++ services |
| `CalculatorJavaClient` | `java_library` | `.jar` | Android apps / Java services |
| `VtsCalculatorServiceTest` | `cc_test` | `/vendor/bin/VtsCalculatorServiceTest` | VTS test runner |

```
aidl_interface "lobo_calculator_aidl"
    ndk backend  →  lobo_calculator_aidl-ndk   (used by cc_binary, cc_library_static, cc_test)
    java backend →  lobo_calculator_aidl-java  (used by java_library)
```

---

### `calculator.rc` — Init Script

**What:** Tells Android's `init` process how to start `calculatord` at boot.

**Why:** `init` is the process manager on Android. Without an `.rc` file, the daemon
is never started, even if the binary is installed.

**How:** Installed via `PRODUCT_COPY_FILES` in `device.mk` to
`/vendor/etc/init/calculator.rc`. Key settings:

```
service calculatord /vendor/bin/calculatord
    class main          ← starts with the main Android boot class
    user system         ← runs as system UID (not root)
    group system
    # no 'oneshot' → init restarts it automatically if it crashes
```

---

### `ICalculator.h` — Abstract C++ Interface

**What:** Pure-virtual C++ interface (`ICalculator`) with four methods.

**Why:** Separates the contract from the Binder implementation. A mock or alternative
implementation (for unit tests) can inherit `ICalculator` without touching Binder.

**How:** `CalculatorServiceImpl` inherits from the AIDL-generated
`BnCalculatorService` (not from `ICalculator` directly), but both express the same
contract. `ICalculator.h` is the human-readable version; `BnCalculatorService` is
the generated version.

---

### `CalculatorServiceImpl.h` / `CalculatorServiceImpl.cpp` — The Implementation

**What:** Concrete class that inherits from `BnCalculatorService` and implements the
four operations.

**Why:** `BnCalculatorService` is the AIDL-generated server stub. Inheriting it means
the Binder driver can dispatch incoming IPC calls to these methods automatically.

**How:** Each method receives the parameters, does the math, writes the result into
`_aidl_return`, and returns `ndk::ScopedAStatus::ok()`. Divide-by-zero returns a
`ServiceSpecificException` with code `ERROR_DIVIDE_BY_ZERO`:

```cpp
ndk::ScopedAStatus CalculatorServiceImpl::divide(int32_t a, int32_t b, int32_t* out) {
    if (b == 0) {
        return ndk::ScopedAStatus::fromServiceSpecificError(
                ICalculatorService::ERROR_DIVIDE_BY_ZERO);
    }
    *out = a / b;
    return ndk::ScopedAStatus::ok();
}
```

---

### `main.cpp` — Daemon Entry Point

**What:** The `main()` function — the process that `init` starts.

**Why:** A Binder service must register itself with `ServiceManager` and then block
waiting for incoming calls. Without `ABinderProcess_joinThreadPool()` the process
would exit immediately.

**How:** Four steps every Binder daemon must do:

```
1. ABinderProcess_setThreadPoolMaxThreadCount(4)   ← allow 4 concurrent calls
2. ABinderProcess_startThreadPool()                ← start the binder thread pool
3. AServiceManager_addService(impl, "service.name") ← register with ServiceManager
4. ABinderProcess_joinThreadPool()                 ← block forever (process incoming calls)
```

---

### `CalculatorClient.h` / `CalculatorClient.cpp` — C++ Client

**What:** A wrapper class that hides Binder connection details from other C++ services.

**Why:** Other services (e.g. FanControlService) should call `client.add(a, b)` — not
deal with `AServiceManager_checkService`, `fromBinder`, `ScopedAStatus` directly.

**How:** `ensureConnected()` lazily looks up the service on first use. If the service
isn't running yet, it logs an error and returns 0. All four operations follow the
same pattern:

```
CalculatorClient::add(a, b)
    → ensureConnected()  → AServiceManager_checkService("com.lobo…")
    → mService->add(a, b, &result)  [Binder IPC]
    → return result
```

---

### `ICalculatorClient.kt` / `CalculatorClientImpl.kt` — Kotlin Client

**What:** Kotlin interface + implementation for Android apps and Java services.

**Why:** Android apps are Java/Kotlin. They cannot use the C++ client library. They
get the same `add/subtract/multiply/divide` API via Kotlin.

**How:** `CalculatorClientImpl` uses `ServiceManager.checkService()` (Android
framework API) to get the Binder, then `ICalculatorService.Stub.asInterface(binder)`
to get the generated Kotlin proxy. The `service` property is initialized lazily via
`by lazy {}`.

```
App calls: CalculatorClientImpl().add(5, 3)
    → ServiceManager.checkService("com.lobo.platform.calculator.ICalculatorService")
    → ICalculatorService.Stub.asInterface(binder)
    → proxy.add(5, 3)  [Binder IPC]
    → returns 8
```

---

### `sepolicy/` — SELinux Policy

**What:** Four files that grant the daemon exactly the permissions it needs — no more.

**Why:** Android enforces SELinux Mandatory Access Control. Without these files,
`calculatord` cannot start, cannot register with ServiceManager, and apps cannot
reach it.

**How — file by file:**

| File | What it defines |
|------|-----------------|
| `calculator.te` | Process domain `calculatord` — allows init to launch it, allows Binder IPC, allows registering as a service |
| `service.te` | Service type `calculatord_service` — marks it as `app_api_service` so untrusted apps can call it |
| `file_contexts` | Labels `/vendor/bin/calculatord` as `calculatord_exec` so init can transition into `calculatord` domain |
| `service_contexts` | Labels the ServiceManager entry `com.lobo.platform.calculator.ICalculatorService` as `calculatord_service` |

The flow: `init` sees `/vendor/bin/calculatord` labelled `calculatord_exec` →
transitions process into `calculatord` domain → `calculator.te` allows it to call
`AServiceManager_addService` → ServiceManager entry gets label `calculatord_service`
from `service_contexts` → `service.te` allows apps to find and call it.

---

### `VtsCalculatorServiceTest.cpp` — VTS Tests

**What:** Vendor Test Suite (VTS) tests that run on the device against the live service.

**Why:** VTS tests verify the real Binder interface on real hardware — not mocks. If
the service is broken on the device, VTS catches it.

**How:**

```
Build:  m VtsCalculatorServiceTest
Run:    atest VtsCalculatorServiceTest
```

Test coverage:
- `add`, `subtract`, `multiply`, `divide` — basic correctness
- Divide-by-zero — verifies `ServiceSpecificException` with `ERROR_DIVIDE_BY_ZERO = 1`
- Concurrency — 8 threads × 100 calls simultaneously (no data races)
- Stability — 1000 repeated calls without errors

---

## Full Runtime Call Flow

```
Boot
 │
 ├── init reads /vendor/etc/init/calculator.rc
 ├── spawns /vendor/bin/calculatord  (SELinux: calculatord_exec → calculatord domain)
 └── calculatord:
       ABinderProcess_startThreadPool()
       AServiceManager_addService(impl, "com.lobo.platform.calculator.ICalculatorService")
       ABinderProcess_joinThreadPool()   ← waiting for calls


App / Service calls add(5, 3):
 │
 ├── [Kotlin] CalculatorClientImpl.add(5, 3)
 │     ServiceManager.checkService("com.lobo.platform.calculator.ICalculatorService")
 │     ICalculatorService.Stub.asInterface(binder)
 │     proxy.add(5, 3)
 │
 ├── [C++]    CalculatorClient::add(5, 3)
 │     AServiceManager_checkService(...)
 │     ICalculatorService::fromBinder(binder)
 │     mService->add(5, 3, &result)
 │
 └── [Binder IPC — kernel driver routes to calculatord thread]
       CalculatorServiceImpl::add(5, 3, &out)
         *out = 5 + 3 = 8
         return ndk::ScopedAStatus::ok()
       → result 8 returned to caller
```

---

## Adding a New Operation

1. Add the method to `ICalculatorService.aidl`
2. Implement it in `CalculatorServiceImpl.cpp`
3. Declare it in `CalculatorServiceImpl.h`
4. Add it to `ICalculator.h`
5. Add the wrapper in `CalculatorClient.cpp` / `CalculatorClient.h`
6. Add the wrapper in `CalculatorClientImpl.kt` / `ICalculatorClient.kt`
7. Add a VTS test in `VtsCalculatorServiceTest.cpp`

---

**Author:** Francis Lobo · **Project:** lobo-aosp-platform
