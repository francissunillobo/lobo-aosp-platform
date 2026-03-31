// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.client.thermalcontrol.impl

import com.lobo.platform.client.thermalcontrol.api.ThermalControlManager

/** Factory for [ThermalControlManager] implementations. */
object ThermalControlManagers {
    @JvmStatic
    fun create(): ThermalControlManager = ThermalControlManagerImpl()
}
