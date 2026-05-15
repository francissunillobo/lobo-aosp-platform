# Bluetooth Design Blueprint

This document defines the final Bluetooth design for `lobo-aosp-platform` with explicit profile coverage (PBAP, MAP, A2DP, AVRCP, HFP), module responsibilities, and a concrete `HalPort` starter API.

---

## What

This design standardizes:

- Module boundaries across `client`, `service`, `hal`, and optional shared `interfaces`.
- Explicit profile-oriented structure for Contacts (PBAP), Messages (MAP), Media (A2DP/AVRCP), and Calls (HFP).
- Separation of app/session orchestration from privileged service logic.
- Separation of service policy logic from hardware/device-specific code.
- A concrete service-to-HAL API (`HalPort`) and callback contract.

---

## Why

Bluetooth in IVI is cross-cutting and long-lived. A clear split reduces regressions and improves portability across devices (RPi5 now, VIM3 later).

Key reasons:

- Managers and Binder connections are client concerns and should not be hosted in privileged service internals.
- Service process should be authoritative for permissions, global arbitration, and contract stability.
- PBAP/MAP/A2DP/AVRCP/HFP are Android-stack capabilities; vendor layer should orchestrate and expose policy-friendly APIs, not reimplement profile stacks.
- HAL implementation is hardware-dependent and should live under `vendor/lobo/hal`, with service using a stable abstraction.

---

## How

### 1) Final folder structure under `lobo-aosp-platform`

```text
lobo-aosp-platform/
└── vendor/
    └── lobo/
        ├── interfaces/                                # optional, recommended for stable shared IPC
        │   └── bluetooth/
        │       └── aidl/src/com/lobo/platform/bluetooth/
        │           ├── IBluetoothService.aidl
        │           ├── IBluetoothCallback.aidl
        │           ├── BtDevice.aidl
        │           ├── PairingState.aidl
        │           └── ConnectionState.aidl
        │
        ├── client/
        │   └── bluetooth/
        │       ├── Android.bp
        │       └── java/src/main/java/com/lobo/platform/client/bluetooth/
        │           ├── api/
        │           │   ├── BluetoothManager.kt
        │           │   ├── BluetoothEventListener.kt
        │           │   ├── profiles/
        │           │   │   ├── A2dpManager.kt
        │           │   │   ├── AvrcpManager.kt
        │           │   │   ├── HfpManager.kt
        │           │   │   ├── PbapManager.kt
        │           │   │   └── MapManager.kt
        │           │   └── model/
        │           │       ├── media/
        │           │       ├── contact/
        │           │       └── message/
        │           ├── impl/
        │           │   ├── BluetoothManagerImpl.kt
        │           │   └── BluetoothManagers.kt
        │           ├── connection/
        │           │   ├── BluetoothServiceConnection.kt
        │           │   ├── BinderDeathRecipient.kt
        │           │   └── BluetoothAidlFacade.kt
        │           ├── repository/
        │           │   ├── BluetoothRepository.kt
        │           │   └── BluetoothRepositoryImpl.kt
        │           └── mapping/
        │               ├── AidlToDomainMapper.kt
        │               └── DomainToAidlMapper.kt
        │
        ├── services/
        │   └── bluetooth/
        │       ├── Android.bp
        │       ├── aidl/com/lobo/platform/bluetooth/
        │       │   ├── IBluetoothService.aidl
        │       │   ├── IAdapterControl.aidl
        │       │   ├── IA2dpControl.aidl
        │       │   ├── IAvrcpControl.aidl
        │       │   ├── IHfpControl.aidl
        │       │   ├── IPbapControl.aidl
        │       │   ├── IMapControl.aidl
        │       │   ├── IAdapterCallback.aidl
        │       │   ├── IA2dpCallback.aidl
        │       │   ├── IPbapCallback.aidl
        │       │   └── IMapCallback.aidl
        │       ├── cpp/include/lobo/platform/service/bluetooth/adapter/
        │       │   ├── IHalPortControl.h
        │       │   ├── IHalPortCallback.h
        │       │   └── ports/
        │       │       ├── IAdapterPortControl.h
        │       │       ├── IA2dpPortControl.h
        │       │       ├── IAvrcpPortControl.h
        │       │       ├── IHfpPortControl.h
        │       │       ├── IPbapPortControl.h
        │       │       └── IMapPortControl.h
        │       └── cpp/core/
        │           ├── include/lobo/platform/bluetooth/
        │           │   ├── BluetoothServiceImpl.h
        │           │   ├── RunBluetoothService.h
        │           │   ├── controllers/*ControlImpl.h
        │           │   └── adapter/
        │           │       ├── HalPortControlFactory.h
        │           │       ├── HalPortControlImpl.h
        │           │       └── ports/*PortControlImpl.h
        │           └── src/
        │               ├── Run.cpp
        │               ├── BluetoothServiceImpl.cpp
        │               ├── controllers/*ControlImpl.cpp
        │               └── adapter/
        │                   ├── HalPortControlFactory.cpp
        │                   ├── HalPortControlImpl.cpp
        │                   └── ports/*PortControlImpl.cpp
        │
        └── hal/
            └── bluetooth/
                ├── Android.bp
                ├── include/
                ├── common/
                ├── rpi5/
                │   └── BluetoothHalAdapter.*          # device-specific implementation
                └── vim3/                              # future target
```

### 2) Responsibility boundaries

- `client/*`: app/session orchestration, UI-friendly state, Binder connection lifecycle.
- `services/bluetooth/service/*`: Binder entrypoint and permission enforcement.
- `services/bluetooth/core/*`: global state machine and cross-client/profile arbitration.
- `services/bluetooth/cpp/core/controllers/*`: Binder-facing profile control implementations.
- `services/bluetooth/cpp/core/adapter/*`: service-side adapter, callback bridge, and HAL port composition.
- `services/bluetooth/cpp/include/.../adapter/ports/*`: profile-wise adapter control contracts.
- `hal/bluetooth/*`: hardware/device-specific implementation details.

### 3) Profile placement and intent

- **A2DP/AVRCP/HFP/PBAP/MAP engines remain in Android stack**.
- Vendor service provides:
  - policy and orchestration,
  - profile-level aggregation for HMI,
  - stable APIs/events for clients.
- Do not duplicate Android profile service internals in vendor modules.

### 4) AIDL placement decision

- Use `vendor/lobo/interfaces/bluetooth` if multiple clients consume the same IPC contract or if API stability/versioning matters.
- Co-locate AIDL in `services/bluetooth` only for early-stage, small-scope, single-consumer features.

### 5) Build-tree mapping reminder

You author in `lobo-aosp-platform/...`.  
The same paths resolve in the synced AOSP tree as `raspi5-aosp/vendor/lobo/...` through your manifest/link setup.

### 6) Verification checklist

- Client reconnects after service death and callbacks recover correctly.
- Per-profile states (A2DP/AVRCP/HFP/PBAP/MAP) are consistent between service logs and HMI.
- Permission checks reject unauthorized callers on every Binder method.
- Multi-client arbitration is deterministic (no policy corruption).
- Device-specific behavior differs only in `vendor/lobo/hal/bluetooth/<target>/`.

### 7) File-by-file responsibilities

#### Interfaces (`vendor/lobo/interfaces/bluetooth/aidl/src/com/lobo/platform/bluetooth/`)

- `IBluetoothService.aidl`: Main IPC contract exposed by vendor Bluetooth service (commands, queries, callback registration).
- `IBluetoothCallback.aidl`: Async callback contract from service to clients for state/event updates.
- `BtDevice.aidl`: Canonical IPC parcelable for remote device identity and attributes.
- `PairingState.aidl`: Pairing/bonding state model passed over IPC.
- `ConnectionState.aidl`: Link/profile connection state model passed over IPC.

#### Client module (`vendor/lobo/client/bluetooth/`)

- `Android.bp`: Declares build targets, AIDL deps, and exported client library artifacts.

Client API (`api/`)
- `BluetoothManager.kt`: Main public entrypoint used by HMI/apps to perform Bluetooth operations.
- `BluetoothEventListener.kt`: Client callback/listener interface for app-layer event handling.
- `profiles/A2dpManager.kt`: Client-facing A2DP-specific operations and state access.
- `profiles/AvrcpManager.kt`: Client-facing AVRCP transport/control operations.
- `profiles/HfpManager.kt`: Client-facing hands-free/call control operations.
- `profiles/PbapManager.kt`: Client-facing phonebook/contact sync operations.
- `profiles/MapManager.kt`: Client-facing message access/sync operations.
- `model/media/*`: Domain models for media/profile states shown in HMI.
- `model/contact/*`: Domain models for contacts/phonebook sync data.
- `model/message/*`: Domain models for MAP/message data and sync states.

Client implementation (`impl/`)
- `BluetoothManagerImpl.kt`: Implements manager APIs, coordinates repositories/connections, and enforces client-side orchestration rules.
- `BluetoothManagers.kt`: Factory/provider for manager instances and dependency wiring.

Connection layer (`connection/`)
- `BluetoothServiceConnection.kt`: Binder service resolution, connect/disconnect lifecycle, and reconnection policy.
- `BinderDeathRecipient.kt`: Handles remote binder death and triggers recovery callbacks.
- `BluetoothAidlFacade.kt`: Typed wrapper over raw AIDL calls to isolate transport details.

Data/repo (`repository/`)
- `BluetoothRepository.kt`: Repository contract for data access and state streams consumed by manager/HMI.
- `BluetoothRepositoryImpl.kt`: Repository implementation combining AIDL facade + local caching/normalization.

Mapping (`mapping/`)
- `AidlToDomainMapper.kt`: Converts AIDL parcelables/enums into client domain models.
- `DomainToAidlMapper.kt`: Converts client domain requests/models into AIDL transport objects.

#### Service module (`vendor/lobo/services/bluetooth/`)

- `Android.bp`: Declares service binaries/libs, AIDL deps, init/sepolicy integration.
- `sepolicy/`: SELinux policy files for service domain, service_contexts, and Binder permissions.
- `init/`: Init `.rc` and startup configuration to launch/register service.

Service boundary (`service/`)
- `BluetoothServiceEntry.kt`: Android service process lifecycle bootstrap and dependency initialization.
- `BluetoothServiceStub.kt`: Concrete Binder stub (`IBluetoothService.Stub`) mapping IPC methods to core orchestration.
- `PermissionEnforcer.kt`: Centralized caller validation and permission enforcement for every Binder entrypoint.

Core orchestration (`core/`)
- `BluetoothOrchestrator.kt`: Authoritative high-level state machine and cross-profile decision point.
- `pairing/PairingCoordinator.kt`: Pairing/bonding flow handling, retries, and timeout sequencing.
- `connection/ConnectionCoordinator.kt`: Connection lifecycle policy (auto-connect, priorities, conflict handling).
- `profiles/A2dpCoordinator.kt`: Service-side A2DP policy orchestration and state aggregation.
- `profiles/AvrcpCoordinator.kt`: Service-side AVRCP policy orchestration and control state handling.
- `profiles/HfpCoordinator.kt`: Service-side HFP orchestration for call audio/control policy.
- `profiles/PbapCoordinator.kt`: Service-side contact sync policy/orchestration for PBAP.
- `profiles/MapCoordinator.kt`: Service-side message access/sync policy/orchestration for MAP.

Adapters (`adapter/`)
- `StackAdapter.kt`: Unified integration point to Android Bluetooth framework/profile proxies.
- `HalPortJniBridge.kt`: Kotlin implementation of `IHalPortControl` that bridges service calls to JNI/native HAL.
- `interface/IHalPortControl.kt`: Service-facing control interface for HAL actions.
- `interface/IHalPortCallback.kt`: Service-facing callback interface for async HAL events.
- `models/HalPortModels.kt`: Shared enums/data models used by control and callback interfaces.
- `profiles/A2dpStackAdapter.kt`: Adapter glue for A2DP stack calls/events.
- `profiles/AvrcpStackAdapter.kt`: Adapter glue for AVRCP stack calls/events.
- `profiles/HfpStackAdapter.kt`: Adapter glue for HFP stack calls/events.
- `profiles/PbapStackAdapter.kt`: Adapter glue for PBAP stack calls/events.
- `profiles/MapStackAdapter.kt`: Adapter glue for MAP stack calls/events.

Session management (`session/`)
- `ClientSessionRegistry.kt`: Tracks connected clients/callbacks, binder tokens, and cleanup on client death.

#### HAL module (`vendor/lobo/hal/bluetooth/`)

- `Android.bp`: Build rules for HAL-facing libraries/binaries and target-specific variants.
- `include/`: Public HAL-facing interfaces/types shared with service adapters.
- `common/`: Target-independent HAL helper code and shared primitives.
- `rpi5/BluetoothHalAdapter.*`: RPi5-specific hardware implementation of `HalPort` contract.
- `vim3/` (future): VIM3-specific implementation with same `HalPort` behavior contract.

### 8) Concrete starter API for `IHalPortControl`

The following starter API is designed for PBAP/MAP/A2DP/AVRCP/HFP orchestration:

```kotlin
interface IHalPortControl {
    fun initialize(callback: IHalPortCallback): Result<Unit>
    fun shutdown(): Result<Unit>

    fun setAdapterEnabled(enabled: Boolean): Result<Unit>
    fun getAdapterState(): AdapterState

    fun startDiscovery(mode: DiscoveryMode = DiscoveryMode.GENERAL): Result<Unit>
    fun stopDiscovery(): Result<Unit>

    fun createBond(address: String, transport: Transport = Transport.AUTO): Result<Unit>
    fun removeBond(address: String): Result<Unit>
    fun setPairingConfirmation(address: String, accept: Boolean): Result<Unit>
    fun setPin(address: String, pin: String): Result<Unit>
    fun setPasskey(address: String, passkey: Int): Result<Unit>

    fun connectProfile(address: String, profile: BtProfile): Result<Unit>
    fun disconnectProfile(address: String, profile: BtProfile): Result<Unit>
    fun getProfileState(address: String, profile: BtProfile): ProfileConnectionState

    fun setA2dpActiveDevice(address: String?): Result<Unit>
    fun getA2dpActiveDevice(): String?
    fun setA2dpCodecPreference(address: String, codec: A2dpCodec): Result<Unit>

    fun sendAvrcpCommand(address: String, command: AvrcpCommand): Result<Unit>
    fun setAbsoluteVolume(address: String, volume: Int): Result<Unit>

    fun setHfpAudioRoute(address: String, enabled: Boolean): Result<Unit>
    fun answerCall(address: String): Result<Unit>
    fun hangupCall(address: String): Result<Unit>
    fun dial(address: String, number: String): Result<Unit>

    fun requestPbapSync(address: String, scope: PbapScope = PbapScope.ALL): Result<Unit>
    fun cancelPbapSync(address: String): Result<Unit>

    fun requestMapFolderList(address: String): Result<Unit>
    fun requestMapMessageList(address: String, folder: String): Result<Unit>
    fun pushMapMessage(address: String, folder: String, message: OutgoingMessage): Result<Unit>
    fun setMapNotificationRegistration(address: String, enabled: Boolean): Result<Unit>
}
```

```kotlin
interface IHalPortCallback {
    fun onAdapterStateChanged(state: AdapterState)
    fun onDiscoveryStateChanged(discovering: Boolean)
    fun onDeviceFound(device: DiscoveredDevice)
    fun onDeviceUpdated(device: DiscoveredDevice)

    fun onBondStateChanged(address: String, state: BondState, reason: FailReason? = null)
    fun onPairingRequest(request: PairingRequest)

    fun onProfileConnectionStateChanged(
        address: String,
        profile: BtProfile,
        state: ProfileConnectionState,
        reason: FailReason? = null
    )

    fun onA2dpActiveDeviceChanged(address: String?)
    fun onA2dpCodecChanged(address: String, codec: A2dpCodec)
    fun onAvrcpPlaybackStateChanged(address: String, state: PlaybackState)
    fun onAvrcpMetadataChanged(address: String, metadata: MediaMetadata)

    fun onHfpAudioStateChanged(address: String, connected: Boolean)
    fun onCallStateChanged(address: String, state: CallState, number: String? = null)

    fun onPbapSyncProgress(address: String, current: Int, total: Int)
    fun onPbapSyncCompleted(address: String, summary: PbapSyncSummary)
    fun onPbapSyncFailed(address: String, reason: FailReason)

    fun onMapFolderList(address: String, folders: List<String>)
    fun onMapMessageList(address: String, folder: String, messages: List<MapMessageSummary>)
    fun onMapMessageReceived(address: String, message: MapMessage)
    fun onMapOperationFailed(address: String, operation: MapOperation, reason: FailReason)

    fun onHalError(error: HalError)
}
```

```kotlin
enum class BtProfile { A2DP, AVRCP, HFP, PBAP, MAP }
enum class AdapterState { OFF, TURNING_ON, ON, TURNING_OFF }
enum class DiscoveryMode { GENERAL, LIMITED }
enum class Transport { AUTO, BR_EDR, LE }
enum class BondState { NONE, BONDING, BONDED }
enum class ProfileConnectionState { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING }
enum class PlaybackState { STOPPED, PAUSED, PLAYING, BUFFERING }
enum class CallState { IDLE, INCOMING, OUTGOING, ACTIVE, HELD }
enum class AvrcpCommand { PLAY, PAUSE, STOP, NEXT, PREVIOUS, FAST_FORWARD, REWIND }
```

Implementation notes:

- `BluetoothOrchestrator` is the primary caller of `IHalPortControl`.
- `IHalPortControl` should be event-driven through `IHalPortCallback` and avoid long blocking calls.
- Keep `IHalPortControl` transport-neutral so the backend can be JNI/C++ today and change later without service-core redesign.

---

## 9) Implemented C++ design status (current)

### What

The current implementation in `vendor/lobo/services/bluetooth` is now C++-first with profile-split AIDL and profile-split service adapters:

- Root service AIDL: `IBluetoothService`
- Profile control AIDLs: `IAdapterControl`, `IA2dpControl`, `IAvrcpControl`, `IHfpControl`, `IPbapControl`, `IMapControl`
- Profile callback AIDLs: `IAdapterCallback`, `IA2dpCallback`, `IPbapCallback`, `IMapCallback`
- Service binder controllers: `*ControlImpl` classes under `cpp/core/src/controllers`
- Service profile ports:
  - Interfaces in `cpp/include/lobo/platform/service/bluetooth/adapter/ports/*.h`
  - Implementations in `cpp/core/src/adapter/ports/*.cpp`
- HAL port bridge:
  - Interface: `IHalPortControl` / `IHalPortCallback`
  - Implementation: `cpp/core/src/adapter/HalPortControlImpl.cpp`
  - Factory: `cpp/core/src/adapter/HalPortControlFactory.cpp`
- HAL/Fluoride skeleton:
  - Contract: `vendor/lobo/hal/bluetooth/include/lobo/platform/hal/*`
  - Concrete split implementation: `vendor/lobo/hal/bluetooth/common/src/FluorideBluetoothHal*.cpp`

### Why

This split preserves modularity while keeping bring-up practical:

- Profile-specific AIDL and ports prevent monolithic interfaces from growing unbounded.
- Controllers stay Binder-facing and thin; profile ports isolate per-profile service behavior.
- `IHalPortControl` keeps service independent from direct HAL class ownership.
- HAL implementation is separated from service and split by profile for easier target bring-up and review.
- `HalPortControlFactory` centralizes backend choice so future target-specific composition is explicit.

### How

Current object graph and call path are assembled as follows:

1. `runBluetoothService()` creates `BluetoothServiceImpl`.
2. `BluetoothServiceImpl` calls `CreateHalPortControl()`.
3. Factory creates `FluorideBluetoothHal` and wraps it in `HalPortControlImpl`.
4. `BluetoothServiceImpl` creates all profile port objects with the shared `IHalPortControl`.
5. `BluetoothServiceImpl` creates all Binder controllers with profile port interfaces.
6. AIDL clients call profile controllers; controllers call profile ports; profile ports call `IHalPortControl`; `HalPortControlImpl` delegates to `IBluetoothHal`.
7. HAL callbacks are registered in `HalPortControlImpl::initialize(...)` and forwarded to `IHalPortCallback`, then dispatched to AIDL callbacks by service controller/session layers.

Known limitations in current code (intentional TODOs while HAL contract catches up):

- `setA2dpCodecPreference` not mapped in `IBluetoothHal` yet.
- `cancelPbapSync` not mapped in `IBluetoothHal` yet.
- `pushMapMessage` not mapped in `IBluetoothHal` yet.
- `setMapNotificationRegistration` not mapped in `IBluetoothHal` yet.

These are tracked as contract-completion tasks between `IHalPortControl` and `IBluetoothHal`.

### Callback threading model (implemented)

- `HalPortControlImpl::CallbackBridge` owns a dedicated worker thread.
- Thread is created at bridge construction using `worker_([this] { run(); })`.
- `on*` bridge methods enqueue callback lambdas into an internal queue (`post(...)`).
- `run()` waits on `queue_cv_`, pops queued jobs, snapshots callback pointer, and executes callback on worker thread.
- Bridge shutdown is orderly (`stopping_` + notify + `join()`), avoiding callback execution after teardown.

### Example sequence (control + callback)

Use `setA2dpActiveDevice(address)` as reference:

1. App calls profile binder method (`IA2dpControl`).
2. Controller delegates to `A2dpPortControlImpl`.
3. Port delegates to `IHalPortControl::setA2dpActiveDevice`.
4. `A2dpFeature` calls `IBluetoothHal::setA2dpActiveDevice`.
5. On success, `A2dpFeature` updates cached active device and posts callback to `CallbackBridge`.
6. Binder call returns immediately after enqueue.
7. CallbackBridge worker thread later executes `IHalPortCallback::onA2dpActiveDeviceChanged`.
8. Service callback fan-out forwards event to registered client callback AIDL.

### Visual diagrams

For class and sequence diagrams, see `BLUETOOTH_ARCHITECTURE.md`:

- Class diagram: implemented C++ layering and ownership/delegation
- Sequence diagram: startup and object wiring
- Sequence diagram: A2DP active device control + async callback
- Sequence diagram: PBAP request + callback path from Fluoride stack
