#pragma once

namespace lobo {
namespace platform {
namespace fancontrol {

struct ThermalConfig {
    float minTempCelsius  = 40.0f;
    float maxTempCelsius  = 80.0f;
    int   pollingIntervalMs = 1000;

    static ThermalConfig loadDefault();
};

} // namespace fancontrol
} // namespace platform
} // namespace lobo
