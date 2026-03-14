package com.lobo.platform.common.types.api

enum class FanState { OFF, LOW, MEDIUM, HIGH, FULL }

data class ThermalZone(
    val name: String,
    val temperatureCelsius: Float,
    val recommendedState: FanState,
)

sealed class Result<out T> {
    data class Success<T>(val value: T) : Result<T>()
    data class Error(val message: String, val cause: Throwable? = null) : Result<Nothing>()
}
