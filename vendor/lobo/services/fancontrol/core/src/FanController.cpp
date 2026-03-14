#include <lobo/platform/fancontrol/FanController.h>
#include <lobo/platform/logging/Logging.h>
#include <string>

namespace lobo {
namespace platform {
namespace fancontrol {

FanController::FanController(int gpioPin)
    : mGpioPin(gpioPin), mCurrentDutyCycle(0) {}

Result FanController::applyDutyCycle(int dutyCycle) {
    if (dutyCycle < 0 || dutyCycle > 100) return Result::INVALID_ARG;
    mCurrentDutyCycle = dutyCycle;
    Logging::log("FanController",
        "GPIO " + std::to_string(mGpioPin) + " duty=" + std::to_string(dutyCycle));
    return Result::OK;
}

int FanController::getCurrentDutyCycle() const { return mCurrentDutyCycle; }

} // namespace fancontrol
} // namespace platform
} // namespace lobo
