// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace lobo {
namespace platform {
namespace hal {

/**
 * One thermal zone sample from the board (e.g. sysfs thermal zone type + temp).
 * Temperature is in millidegrees Celsius (Linux sysfs convention).
 */
struct ThermalZoneSample {
    std::string zone_name;
    int64_t temp_millicelsius = 0;
};

/**
 * Board-agnostic contract for reading thermal zones.
 * Implementations live under vendor/lobo/hal/<board>/temperature_monitor/.
 */
class ITemperatureMonitorHal {
public:
    virtual ~ITemperatureMonitorHal() = default;

    /** Fills @p out with current samples; clears @p out first. Returns false on fatal I/O error. */
    virtual bool readSamples(std::vector<ThermalZoneSample>& out) = 0;
};

} // namespace hal
} // namespace platform
} // namespace lobo
