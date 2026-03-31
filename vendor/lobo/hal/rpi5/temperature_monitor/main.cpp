// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

// RPi5 product entry: links the sysfs HAL + shared daemon core.
// Other boards (e.g. Khadas VIM3) add vendor/lobo/hal/<board>/temperature_monitor/
// with temperature_monitord_<board> + stem "temperature_monitord".

#include <lobo/platform/hal/rpi5/TemperatureMonitorHalRpi5.h>
#include <lobo/platform/temperaturemonitor/RunTemperatureMonitor.h>

#include <memory>

int main() {
    return lobo::platform::temperaturemonitor::runTemperatureMonitorService(
            std::make_unique<lobo::platform::hal::rpi5::TemperatureMonitorHalRpi5>());
}
