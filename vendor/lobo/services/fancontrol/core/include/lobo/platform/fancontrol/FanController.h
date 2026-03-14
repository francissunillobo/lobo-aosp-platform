#pragma once

#include <lobo/platform/types/CommonTypes.h>

namespace lobo {
namespace platform {
namespace fancontrol {

class FanController {
public:
    explicit FanController(int gpioPin);

    Result applyDutyCycle(int dutyCycle);
    int    getCurrentDutyCycle() const;

private:
    int mGpioPin;
    int mCurrentDutyCycle;
};

} // namespace fancontrol
} // namespace platform
} // namespace lobo
