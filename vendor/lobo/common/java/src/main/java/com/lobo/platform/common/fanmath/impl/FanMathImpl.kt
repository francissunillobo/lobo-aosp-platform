package com.lobo.platform.common.fanmath.impl

import com.lobo.platform.common.fanmath.api.FanMath

class FanMathImpl : FanMath {
    override fun tempToDutyCycle(tempCelsius: Float, minTemp: Float, maxTemp: Float): Int {
        if (tempCelsius <= minTemp) return 0
        if (tempCelsius >= maxTemp) return 100
        return ((tempCelsius - minTemp) / (maxTemp - minTemp) * 100.0f).toInt()
    }

    override fun clamp(value: Int, min: Int, max: Int): Int = value.coerceIn(min, max)
}
