package com.lobo.platform.common.fanmath.api

interface FanMath {
    fun tempToDutyCycle(tempCelsius: Float, minTemp: Float, maxTemp: Float): Int
    fun clamp(value: Int, min: Int, max: Int): Int
}
