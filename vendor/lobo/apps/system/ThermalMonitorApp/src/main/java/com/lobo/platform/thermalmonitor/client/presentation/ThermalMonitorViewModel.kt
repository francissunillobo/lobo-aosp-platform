// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.thermalmonitor.client.presentation

import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.lobo.platform.client.thermalcontrol.api.ThermalReadingsListener
import com.lobo.platform.client.thermalcontrol.impl.ThermalControlManagers
import com.lobo.platform.temperaturemonitor.ThermalZoneReading
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

class ThermalMonitorViewModel : ViewModel() {

    private companion object {
        private const val TAG = "ThermalMonitorVM"
    }

    private val manager = ThermalControlManagers.create()
    private var callbackFallbackJob: Job? = null
    /** Any successful zones update (callback or pull) — avoids false "stale" when daemon skips notify. */
    @Volatile
    private var lastZonesUpdateAtMs: Long = 0L

    private val readingsListener =
        ThermalReadingsListener { readings ->
            viewModelScope.launch(Dispatchers.Main.immediate) {
                applyReadings(readings)
            }
        }

    private val _uiState = MutableStateFlow(ThermalUiState())
    val uiState: StateFlow<ThermalUiState> = _uiState.asStateFlow()

    init {
        connectAndSubscribe()
    }

    private fun connectAndSubscribe() {
        if (!manager.connect()) {
            _uiState.update {
                it.copy(
                    connected = false,
                    errorMessage = "temperature_monitord not available (check ServiceManager).",
                )
            }
            return
        }
        _uiState.update { it.copy(connected = true, errorMessage = null) }
        refreshDaemonSettingsAndReadings()
        manager.registerListener(readingsListener)
        startCallbackFallbackLoop()
    }

    /** Push updated poll interval / notify threshold to the daemon and refresh local state. */
    fun applyDaemonSettings(pollIntervalMillis: Long?, notifyThresholdMilliCelsius: Long?) {
        if (!manager.isConnected) {
            return
        }
        pollIntervalMillis?.let { manager.setPollIntervalMillis(it) }
        notifyThresholdMilliCelsius?.let { manager.setNotifyThresholdMilliCelsius(it) }
        // IMPORTANT: after pressing Apply we update only the daemon settings
        // (poll interval + notify threshold) but do not force a zones snapshot pull.
        // This makes it obvious whether `onReadingsChanged()` callbacks are truly
        // firing based on notify threshold (zones updates should then be callback-driven).
        refreshDaemonSettingsOnly()
    }

    private fun refreshDaemonSettingsOnly() {
        val poll = manager.getPollIntervalMillis()
        val threshold = manager.getNotifyThresholdMilliCelsius()
        _uiState.update { state ->
            state.copy(
                pollIntervalMillis = poll,
                notifyThresholdMilliCelsius = threshold,
            )
        }
        Log.i(TAG, "daemon settings: poll=$poll threshold=$threshold")
    }

    private fun refreshDaemonSettingsAndReadings() {
        refreshDaemonSettingsOnly()
        val readings = manager.getCurrentReadings()
        lastZonesUpdateAtMs = System.currentTimeMillis()
        _uiState.update { state ->
            state.copy(zones = readings.toRows())
        }
    }

    private fun applyReadings(readings: Array<ThermalZoneReading>) {
        // Callback-driven update: used to verify notify-threshold behavior.
        lastZonesUpdateAtMs = System.currentTimeMillis()
        if (readings.isNotEmpty()) {
            Log.i(TAG, "callback: zones=${readings.size} first=${readings[0].zoneName}:${readings[0].tempMilliCelsius}")
        } else {
            Log.i(TAG, "callback: zones=0")
        }
        _uiState.update { it.copy(zones = readings.toList().map { z -> z.toRow() }) }
    }

    private fun startCallbackFallbackLoop() {
        callbackFallbackJob?.cancel()
        callbackFallbackJob =
            viewModelScope.launch {
                while (isActive) {
                    delay(2500L)
                    if (!manager.isConnected) {
                        continue
                    }
                    val now = System.currentTimeMillis()
                    // Daemon may skip notify for >5s when change is below threshold; use a longer
                    // "stale" window and track any successful UI update (callback or pull).
                    val staleMs = 15000L
                    if (now - lastZonesUpdateAtMs > staleMs) {
                        val readings = manager.getCurrentReadings()
                        _uiState.update { it.copy(zones = readings.toRows()) }
                        lastZonesUpdateAtMs = now
                        Log.w(
                            TAG,
                            "fallback pull refresh: no zones update in >${staleMs / 1000}s (callbacks may be quiet when below notify threshold)",
                        )
                    }
                }
            }
    }

    override fun onCleared() {
        callbackFallbackJob?.cancel()
        manager.unregisterListener(readingsListener)
        manager.disconnect()
        super.onCleared()
    }
}

private fun Array<ThermalZoneReading>?.toRows(): List<ThermalZoneRow> =
    this?.map { it.toRow() }.orEmpty()

private fun ThermalZoneReading.toRow(): ThermalZoneRow =
    ThermalZoneRow(
        zoneName = zoneName.orEmpty(),
        tempMilliCelsius = tempMilliCelsius,
        timestampNanos = timestampNanos,
    )
