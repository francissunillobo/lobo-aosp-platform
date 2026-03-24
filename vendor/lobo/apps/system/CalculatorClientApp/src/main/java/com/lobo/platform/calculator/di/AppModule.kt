// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.calculator.di

import com.lobo.platform.calculator.api.ICalculatorClient
import com.lobo.platform.calculator.impl.CalculatorClientImpl

object AppModule {
    fun provideCalculatorClient(): ICalculatorClient = CalculatorClientImpl()
}
