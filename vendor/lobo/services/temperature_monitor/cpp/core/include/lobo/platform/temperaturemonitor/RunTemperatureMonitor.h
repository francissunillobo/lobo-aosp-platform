// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <lobo/platform/hal/ITemperatureMonitorHal.h>

#include <memory>

namespace lobo {
namespace platform {
namespace temperaturemonitor {

/**
 * Registers the Binder service and blocks in the Binder thread pool.
 * Each product links a small main() that constructs the board-specific
 * ITemperatureMonitorHal and passes it here.
 */
int runTemperatureMonitorService(
        std::unique_ptr<lobo::platform::hal::ITemperatureMonitorHal> hal);

} // namespace temperaturemonitor
} // namespace platform
} // namespace lobo
