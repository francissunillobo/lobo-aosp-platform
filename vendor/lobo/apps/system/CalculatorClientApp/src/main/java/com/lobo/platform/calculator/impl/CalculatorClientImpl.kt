// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.calculator.impl

import android.os.ServiceManager
import android.util.Log
import com.lobo.platform.calculator.api.ICalculatorClient
import com.lobo.platform.calculator.ICalculatorService

/**
 * Kotlin implementation of ICalculatorClient.
 * Connects to the calculatord Binder service and delegates all calls to it.
 */
class CalculatorClientImpl : ICalculatorClient {

    private val tag = "CalculatorClientImpl"
    private val serviceName = "com.lobo.platform.calculator.ICalculatorService"

    private val service: ICalculatorService? by lazy {
        val binder = ServiceManager.checkService(serviceName)
        if (binder == null) {
            Log.e(tag, "Service '$serviceName' not found")
            null
        } else {
            ICalculatorService.Stub.asInterface(binder)
        }
    }

    override fun add(a: Int, b: Int): Int =
        service?.add(a, b) ?: 0.also { Log.e(tag, "add: service unavailable") }

    override fun subtract(a: Int, b: Int): Int =
        service?.subtract(a, b) ?: 0.also { Log.e(tag, "subtract: service unavailable") }

    override fun multiply(a: Int, b: Int): Int =
        service?.multiply(a, b) ?: 0.also { Log.e(tag, "multiply: service unavailable") }

    override fun divide(a: Int, b: Int): Int =
        service?.divide(a, b) ?: 0.also { Log.e(tag, "divide: service unavailable") }
}
