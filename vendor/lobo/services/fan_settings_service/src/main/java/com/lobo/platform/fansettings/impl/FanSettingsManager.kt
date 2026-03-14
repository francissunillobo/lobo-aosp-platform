package com.lobo.platform.fansettings.impl

import com.lobo.platform.fansettings.api.FanSettingsContract

class FanSettingsManager : FanSettingsContract {
    private var maxTemp: Int = 80
    private var minTemp: Int = 40
    private var autoMode: Boolean = true

    override fun getMaxTemperature(): Int = maxTemp
    override fun setMaxTemperature(tempCelsius: Int) { maxTemp = tempCelsius }
    override fun getMinTemperature(): Int = minTemp
    override fun setMinTemperature(tempCelsius: Int) { minTemp = tempCelsius }
    override fun isAutoMode(): Boolean = autoMode
    override fun setAutoMode(enabled: Boolean) { autoMode = enabled }
}
