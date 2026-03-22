// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <aidl/com/lobo/platform/calculator/BnCalculatorService.h>

namespace lobo {
namespace platform {
namespace calculator {

/**
 * Binder implementation of ICalculatorService.
 * Inherits from the AIDL-generated BnCalculatorService stub.
 * Registered with the system ServiceManager in main.cpp.
 */
class CalculatorServiceImpl
        : public aidl::com::lobo::platform::calculator::BnCalculatorService {
public:
    ndk::ScopedAStatus add(int32_t a, int32_t b, int32_t* _aidl_return) override;
    ndk::ScopedAStatus subtract(int32_t a, int32_t b, int32_t* _aidl_return) override;
    ndk::ScopedAStatus multiply(int32_t a, int32_t b, int32_t* _aidl_return) override;
    ndk::ScopedAStatus divide(int32_t a, int32_t b, int32_t* _aidl_return) override;
};

} // namespace calculator
} // namespace platform
} // namespace lobo
