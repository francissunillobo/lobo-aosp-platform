#include <lobo/platform/hal/rpi5/FanHalRpi5.h>
#include <lobo/platform/logging/Logging.h>
#include <fstream>
#include <string>

namespace lobo {
namespace platform {
namespace rpi5 {

FanHalRpi5::FanHalRpi5(int gpioPin)
    : mGpioPin(gpioPin), mCurrentState(FanState::OFF) {}

FanHalRpi5::~FanHalRpi5() {
    setFanState(FanState::OFF);
}

Result FanHalRpi5::setFanState(FanState state) {
    mCurrentState = state;
    Logging::log("FanHalRpi5",
        "GPIO " + std::to_string(mGpioPin) + " state=" + std::to_string(static_cast<int>(state)));
    return Result::OK;
}

FanState FanHalRpi5::getFanState() const {
    return mCurrentState;
}

float FanHalRpi5::readTemperatureCelsius() {
    // Read from Raspberry Pi thermal zone sysfs
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    int millidegrees = 0;
    if (f.is_open()) f >> millidegrees;
    return millidegrees / 1000.0f;
}

} // namespace rpi5
} // namespace platform
} // namespace lobo
