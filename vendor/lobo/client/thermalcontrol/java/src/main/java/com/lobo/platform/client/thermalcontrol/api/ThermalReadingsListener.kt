// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.client.thermalcontrol.api

import com.lobo.platform.temperaturemonitor.ThermalZoneReading

/** Push updates from [ThermalControlManager] when zone readings change (daemon-side threshold). */
fun interface ThermalReadingsListener {
    fun onReadingsChanged(readings: Array<ThermalZoneReading>)
}
