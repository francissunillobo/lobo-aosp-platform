// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#define LOG_TAG "calculatord"

#include <lobo/platform/calculator/CalculatorServiceImpl.h>
#include <log/log.h>

namespace lobo {
namespace platform {
namespace calculator {

ndk::ScopedAStatus CalculatorServiceImpl::add(
        int32_t a, int32_t b, int32_t* _aidl_return) {
    *_aidl_return = a + b;
    ALOGD("add(%d, %d) = %d", a, b, *_aidl_return);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus CalculatorServiceImpl::subtract(
        int32_t a, int32_t b, int32_t* _aidl_return) {
    *_aidl_return = a - b;
    ALOGD("subtract(%d, %d) = %d", a, b, *_aidl_return);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus CalculatorServiceImpl::multiply(
        int32_t a, int32_t b, int32_t* _aidl_return) {
    *_aidl_return = a * b;
    ALOGD("multiply(%d, %d) = %d", a, b, *_aidl_return);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus CalculatorServiceImpl::divide(
        int32_t a, int32_t b, int32_t* _aidl_return) {
    if (b == 0) {
        ALOGE("divide(%d, 0): division by zero", a);
        return ndk::ScopedAStatus::fromServiceSpecificError(
                ICalculatorService::ERROR_DIVIDE_BY_ZERO);
    }
    *_aidl_return = a / b;
    ALOGD("divide(%d, %d) = %d", a, b, *_aidl_return);
    return ndk::ScopedAStatus::ok();
}

} // namespace calculator
} // namespace platform
} // namespace lobo
