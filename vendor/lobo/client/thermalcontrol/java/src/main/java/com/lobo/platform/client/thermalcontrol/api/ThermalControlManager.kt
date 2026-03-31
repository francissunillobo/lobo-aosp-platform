// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.client.thermalcontrol.api

import com.lobo.platform.temperaturemonitor.ThermalZoneReading

/**
 * Binder façade for vendor thermal monitoring (`temperature_monitord`) and, later, fan control.
 * Create instances with [com.lobo.platform.client.thermalcontrol.impl.ThermalControlManagers.create].
 */
interface ThermalControlManager {

    /** True after [connect] succeeds until [disconnect]. */
    val isConnected: Boolean

    /**
     * Resolves [ITemperatureMonitorService] via ServiceManager and wraps the IBinder for
     * vendor-private Binder (Treble).
     */
    fun connect(): Boolean

    fun disconnect()

    fun getCurrentReadings(): Array<ThermalZoneReading>?

    fun registerListener(listener: ThermalReadingsListener)

    fun unregisterListener(listener: ThermalReadingsListener)

    fun getPollIntervalMillis(): Long

    fun setPollIntervalMillis(intervalMs: Long)

    fun getNotifyThresholdMilliCelsius(): Long

    fun setNotifyThresholdMilliCelsius(thresholdMilli: Long)
}
