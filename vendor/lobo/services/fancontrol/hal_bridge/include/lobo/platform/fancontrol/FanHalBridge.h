#pragma once

#include <lobo/platform/types/CommonTypes.h>

namespace lobo {
namespace platform {
namespace fancontrol {

// Stub bridge between the service layer and the board HAL.
// TODO: inject IFanHal via AIDL when HAL interfaces include path is wired up.
class FanHalBridge {
public:
    FanHalBridge() = default;

    Result applyState(FanState state);
    FanState getCurrentState() const;

private:
    FanState mCurrentState{FanState::OFF};
};

} // namespace fancontrol
} // namespace platform
} // namespace lobo
