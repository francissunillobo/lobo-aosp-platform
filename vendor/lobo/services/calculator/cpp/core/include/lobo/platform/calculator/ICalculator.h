// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace lobo {
namespace platform {
namespace calculator {

/**
 * Abstract C++ interface for the calculator service.
 * Separates the contract from the implementation.
 * Any alternative implementation (e.g. for testing) must inherit this.
 */
class ICalculator {
public:
    virtual ~ICalculator() = default;

    virtual int32_t add(int32_t a, int32_t b) = 0;
    virtual int32_t subtract(int32_t a, int32_t b) = 0;
    virtual int32_t multiply(int32_t a, int32_t b) = 0;

    /**
     * Divide a by b.
     * Returns the integer quotient (truncates toward zero).
     * Behaviour when b == 0 is defined by the implementation
     * (CalculatorServiceImpl returns an error via Binder).
     */
    virtual int32_t divide(int32_t a, int32_t b) = 0;
};

} // namespace calculator
} // namespace platform
} // namespace lobo
