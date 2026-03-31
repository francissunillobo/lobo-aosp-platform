// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <lobo/platform/hal/ITemperatureMonitorHal.h>

namespace lobo {
namespace platform {
namespace hal {
namespace rpi5 {

/** Raspberry Pi 5 (Linux) sysfs thermal zones under /sys/class/thermal. */
class TemperatureMonitorHalRpi5 : public ITemperatureMonitorHal {
public:
    bool readSamples(std::vector<ThermalZoneSample>& out) override;
};

} // namespace rpi5
} // namespace hal
} // namespace platform
} // namespace lobo
