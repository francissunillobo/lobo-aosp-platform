// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#define LOG_TAG "calculatord"

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <log/log.h>

#include <lobo/platform/calculator/CalculatorServiceImpl.h>

static constexpr const char* kServiceName =
        "com.lobo.platform.calculator.ICalculatorService";

int main() {
    ALOGI("calculatord starting");

    // Allow up to 4 concurrent binder threads.
    // Each incoming call (add/subtract/multiply/divide) runs on one thread.
    ABinderProcess_setThreadPoolMaxThreadCount(4);
    ABinderProcess_startThreadPool();

    auto service = ndk::SharedRefBase::make<
            lobo::platform::calculator::CalculatorServiceImpl>();

    binder_status_t status = AServiceManager_addService(
            service->asBinder().get(), kServiceName);

    if (status != STATUS_OK) {
        ALOGE("addService('%s') failed: %d", kServiceName, status);
        return 1;
    }

    ALOGI("calculatord registered as '%s'", kServiceName);

    // Block forever — process incoming binder calls until the system kills us.
    ABinderProcess_joinThreadPool();
    return 0;
}
