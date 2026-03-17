#include <lobo/platform/fancontrol/FanHalBridge.h>

namespace lobo {
namespace platform {
namespace fancontrol {

Result FanHalBridge::applyState(FanState state) {
    mCurrentState = state;
    return Result::OK;
}

FanState FanHalBridge::getCurrentState() const { return mCurrentState; }

} // namespace fancontrol
} // namespace platform
} // namespace lobo
