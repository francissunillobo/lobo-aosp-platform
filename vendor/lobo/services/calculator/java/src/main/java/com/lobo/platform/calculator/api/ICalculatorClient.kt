// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.calculator.api

/**
 * Kotlin interface for the calculator service client.
 * Hides Binder details from Java/Kotlin callers.
 */
interface ICalculatorClient {
    fun add(a: Int, b: Int): Int
    fun subtract(a: Int, b: Int): Int
    fun multiply(a: Int, b: Int): Int

    /**
     * Divide a by b.
     * Returns 0 if b == 0 or the service is unavailable.
     */
    fun divide(a: Int, b: Int): Int
}
