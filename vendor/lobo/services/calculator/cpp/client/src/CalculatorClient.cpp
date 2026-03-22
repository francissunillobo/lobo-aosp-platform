// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#define LOG_TAG "CalculatorClient"

#include <lobo/platform/calculator/CalculatorClient.h>
#include <android/binder_manager.h>
#include <log/log.h>

namespace lobo {
namespace platform {
namespace calculator {

static constexpr const char* kServiceName =
        "com.lobo.platform.calculator.ICalculatorService";

CalculatorClient::CalculatorClient() = default;

bool CalculatorClient::ensureConnected() {
    if (mService) return true;
    ndk::SpAIBinder binder(AServiceManager_checkService(kServiceName));
    if (binder.get() == nullptr) {
        ALOGE("CalculatorClient: service '%s' not found", kServiceName);
        return false;
    }
    mService = aidl::com::lobo::platform::calculator::ICalculatorService::fromBinder(binder);
    return mService != nullptr;
}

int32_t CalculatorClient::add(int32_t a, int32_t b) {
    if (!ensureConnected()) return 0;
    int32_t result = 0;
    auto status = mService->add(a, b, &result);
    if (!status.isOk()) ALOGE("add failed: %s", status.getDescription().c_str());
    return result;
}

int32_t CalculatorClient::subtract(int32_t a, int32_t b) {
    if (!ensureConnected()) return 0;
    int32_t result = 0;
    auto status = mService->subtract(a, b, &result);
    if (!status.isOk()) ALOGE("subtract failed: %s", status.getDescription().c_str());
    return result;
}

int32_t CalculatorClient::multiply(int32_t a, int32_t b) {
    if (!ensureConnected()) return 0;
    int32_t result = 0;
    auto status = mService->multiply(a, b, &result);
    if (!status.isOk()) ALOGE("multiply failed: %s", status.getDescription().c_str());
    return result;
}

int32_t CalculatorClient::divide(int32_t a, int32_t b) {
    if (!ensureConnected()) return 0;
    int32_t result = 0;
    auto status = mService->divide(a, b, &result);
    if (!status.isOk()) ALOGE("divide failed: %s", status.getDescription().c_str());
    return result;
}

} // namespace calculator
} // namespace platform
} // namespace lobo
