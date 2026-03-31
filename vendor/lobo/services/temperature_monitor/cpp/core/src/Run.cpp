// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#define LOG_TAG "temperature_monitord"

#include <lobo/platform/temperaturemonitor/RunTemperatureMonitor.h>
#include <lobo/platform/temperaturemonitor/TemperatureMonitorServiceImpl.h>

#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <log/log.h>

namespace {

constexpr const char* kServiceName =
        "com.lobo.platform.temperaturemonitor.ITemperatureMonitorService";

} // namespace

namespace lobo {
namespace platform {
namespace temperaturemonitor {

int runTemperatureMonitorService(
        std::unique_ptr<lobo::platform::hal::ITemperatureMonitorHal> hal) {
    ALOGI("temperature_monitord starting");

    ABinderProcess_setThreadPoolMaxThreadCount(4);
    ABinderProcess_startThreadPool();

    auto service = ndk::SharedRefBase::make<TemperatureMonitorServiceImpl>(std::move(hal));

    const binder_status_t status =
            AServiceManager_addService(service->asBinder().get(), kServiceName);

    if (status != STATUS_OK) {
        ALOGE("addService('%s') failed: %d", kServiceName, status);
        return 1;
    }

    ALOGI("temperature_monitord registered as '%s'", kServiceName);

    ABinderProcess_joinThreadPool();
    return 0;
}

} // namespace temperaturemonitor
} // namespace platform
} // namespace lobo
