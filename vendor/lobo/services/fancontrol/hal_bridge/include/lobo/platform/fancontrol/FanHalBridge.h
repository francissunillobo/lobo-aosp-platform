#pragma once

#include <lobo/platform/hal/IFanHal.h>
#include <lobo/platform/types/CommonTypes.h>
#include <memory>

namespace lobo {
namespace platform {
namespace fancontrol {

// Bridge between the service layer and the board HAL.
// Depends ONLY on IFanHal — never on a board-specific implementation.
class FanHalBridge {
public:
    explicit FanHalBridge(std::shared_ptr<lobo::platform::IFanHal> hal);

    Result applyState(FanState state);
    FanState getCurrentState() const;

private:
    std::shared_ptr<lobo::platform::IFanHal> mHal;
    FanState mCurrentState;
};

} // namespace fancontrol
} // namespace platform
} // namespace lobo
