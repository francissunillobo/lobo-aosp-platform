#include <lobo/platform/fancontrol/ThermalPolicy.h>

namespace lobo {
namespace platform {
namespace fancontrol {

ThermalPolicy::ThermalPolicy(float minTemp, float maxTemp)
    : mMinTemp(minTemp), mMaxTemp(maxTemp) {}

FanState ThermalPolicy::evaluate(float temp) const {
    if (temp < mMinTemp)                              return FanState::OFF;
    if (temp < mMinTemp + (mMaxTemp - mMinTemp) * 0.25f) return FanState::LOW;
    if (temp < mMinTemp + (mMaxTemp - mMinTemp) * 0.50f) return FanState::MEDIUM;
    if (temp < mMinTemp + (mMaxTemp - mMinTemp) * 0.75f) return FanState::HIGH;
    return FanState::FULL;
}

} // namespace fancontrol
} // namespace platform
} // namespace lobo
