// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.thermalmonitor.client.presentation

import com.lobo.platform.temperaturemonitor.ThermalZoneReading

/** UI model for thermal zone rows (mirrors [ThermalZoneReading], stable for list/text display). */
data class ThermalZoneRow(
    val zoneName: String,
    val tempMilliCelsius: Long,
    val timestampNanos: Long,
)

/**
 * Single screen state for the thermal monitor: connection, optional error, daemon settings,
 * and last known zone readings (from push or initial fetch).
 */
data class ThermalUiState(
    val connected: Boolean = false,
    val errorMessage: String? = null,
    val pollIntervalMillis: Long = 0L,
    val notifyThresholdMilliCelsius: Long = 0L,
    val zones: List<ThermalZoneRow> = emptyList(),
)
