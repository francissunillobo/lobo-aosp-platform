// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <aidl/com/lobo/platform/calculator/ICalculatorService.h>
#include <cstdint>
#include <memory>

namespace lobo {
namespace platform {
namespace calculator {

/**
 * C++ client wrapper for ICalculatorService.
 *
 * Hides Binder connection details from callers.
 * Other C++ services (e.g. FanControlService) use this class
 * instead of talking to the Binder proxy directly.
 *
 * Usage:
 *   CalculatorClient client;
 *   int32_t result = client.add(5, 3);  // returns 8
 */
class CalculatorClient {
public:
    CalculatorClient();

    int32_t add(int32_t a, int32_t b);
    int32_t subtract(int32_t a, int32_t b);
    int32_t multiply(int32_t a, int32_t b);

    /**
     * Divide a by b.
     * Returns 0 and logs an error if b == 0 or the service is unavailable.
     */
    int32_t divide(int32_t a, int32_t b);

private:
    std::shared_ptr<aidl::com::lobo::platform::calculator::ICalculatorService> mService;

    /** Lazily connects to the service on first use. Returns false if unavailable. */
    bool ensureConnected();
};

} // namespace calculator
} // namespace platform
} // namespace lobo
