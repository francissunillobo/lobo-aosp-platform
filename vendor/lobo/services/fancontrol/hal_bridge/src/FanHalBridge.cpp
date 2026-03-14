#include <lobo/platform/fancontrol/FanHalBridge.h>
#include <lobo/platform/logging/Logging.h>

namespace lobo {
namespace platform {
namespace fancontrol {

FanHalBridge::FanHalBridge(std::shared_ptr<lobo::platform::IFanHal> hal)
    : mHal(std::move(hal)), mCurrentState(FanState::OFF) {}

Result FanHalBridge::applyState(FanState state) {
    Result r = mHal->setFanState(state);
    if (r == Result::OK) mCurrentState = state;
    return r;
}

FanState FanHalBridge::getCurrentState() const { return mCurrentState; }

} // namespace fancontrol
} // namespace platform
} // namespace lobo
