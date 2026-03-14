#pragma once

#include <lobo/platform/types/CommonTypes.h>

namespace lobo {
namespace platform {
namespace fancontrol {

class ThermalPolicy {
public:
    explicit ThermalPolicy(float minTemp, float maxTemp);

    FanState evaluate(float currentTempCelsius) const;

private:
    float mMinTemp;
    float mMaxTemp;
};

} // namespace fancontrol
} // namespace platform
} // namespace lobo
