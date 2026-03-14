package com.lobo.platform.fansettings.api

interface FanSettingsContract {
    fun getMaxTemperature(): Int
    fun setMaxTemperature(tempCelsius: Int)
    fun getMinTemperature(): Int
    fun setMinTemperature(tempCelsius: Int)
    fun isAutoMode(): Boolean
    fun setAutoMode(enabled: Boolean)
}
